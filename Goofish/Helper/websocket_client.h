#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <deque>

namespace websocket_chat {

/**
 * @brief 简单封装的 WebSocket 客户端（支持 plain / TLS）
 *
 * 该类在内部使用 boost::asio/io_context 和单独的 IO 线程进行异步操作，
 * 提供回调以通知消息、错误、连接与断开事件。发送使用内部队列并在 strand 中序列化。
 */
class WebSocketClient {
public:
    using MessageCallback = std::function<void(const std::string&, bool)>; 
    using ErrorCallback = std::function<void(const std::string&)>; 
    using ConnectionCallback = std::function<void()>;
    using DisconnectionCallback = std::function<void()>;

    /**
     * @brief 构造函数，初始化 IO 与 SSL 上下文相关成员
     */
    WebSocketClient();
    /**
     * @brief 析构函数，确保资源清理并停止 IO 线程
     */
    ~WebSocketClient();

    /**
     * @brief 建立到服务器的同步连接并完成 WebSocket 握手
     *
     * @param host     服务器主机名或 IP（用于握手中的 Host 头）
     * @param port     服务器端口（例如 "80"、"443" 或 "8080"）
     * @param target   WebSocket 路径（例如 "/" 或 "/chat"）
     * @param verify_peer 是否校验证书链（仅在 use_ssl 时有效，默认 true）
     * @param ca_file   可选：自定义 CA bundle 的路径；为空时使用系统默认
     * @return true 如果连接与握手成功并且 IO 线程启动；false 表示失败（并回调错误）
     *
     * 说明：此函数会选择 plain 或 TLS 流，成功后会启动内部 IO 线程处理异步读写。
     */
    bool connect(const std::string& host, const std::string& port, const std::string& target,
                 bool verify_peer = true, const std::string& ca_file = "");
    
    /**
     * @brief 发送文本消息（异步）
     *
     * @param message 文本内容（UTF-8）
     * @return true 表示消息已成功入队等待发送；false 表示当前未连接或入队失败
     *
     * 说明：实际写操作由内部写队列与 strand 序列化执行。
     */
    bool SendText(const std::string& message);

    /**
     * @brief 发送二进制消息（异步）
     *
     * @param data 二进制数据缓冲
     * @return true 表示数据已成功入队等待发送；false 表示当前未连接或入队失败
     */
    bool send_binary(const std::vector<uint8_t>& data);

    /**
     * @brief 主动断开连接（异步关闭）
     *
     * 该调用会触发关闭流程并在适当时通知断开回调。
     */
    void disconnect();

    /**
     * @brief 查询当前连接状态
     *
     * @return true 如果已连接（handshake 成功并未断开）
     */
    bool is_connected() const { return connected_.load(std::memory_order_acquire); }
    
    /**
     * @brief 设置接收到消息时的回调
     *
     * 回调签名：void(const std::string& payload, bool is_binary)
     * - payload: 接收到的消息数据（文本或二进制以字符串承载）
     * - is_binary: 是否为二进制消息
     */
    void set_message_callback(MessageCallback callback) { message_callback_ = std::move(callback); }

    /**
     * @brief 设置错误回调（发生网络/协议错误时调用）
     *
     * 回调签名：void(const std::string& error_message)
     */
    void set_error_callback(ErrorCallback callback) { error_callback_ = std::move(callback); }

    /**
     * @brief 设置连接成功后的回调（握手完成时调用）
     */
    void set_connection_callback(ConnectionCallback callback) { connection_callback_ = std::move(callback); }

    /**
     * @brief 设置断开连接回调（远端断开或主动关闭完成时调用）
     */
    void set_disconnection_callback(DisconnectionCallback callback) { disconnection_callback_ = std::move(callback); }
    
    /**
     * @brief 启动读取循环（通常内部在 connect 成功后自动调用）
     *
     * 公开此接口以支持在特定场景下手动启动读取任务。
     */
    void start_read_loop();

private:
    /**
     * @brief 读取处理回调：处理单次异步读完成事件
     *
     * @param ec      读操作结果错误码
     * @param bytes_transferred 已读取字节数（供调试/日志使用）
     *
     * 说明：此函数解析 buffer_ 并通过 message_callback_ 转发消息，必要时继续异步读取。
     */
    void handle_message(boost::beast::error_code ec, std::size_t bytes_transferred);

    /**
     * @brief 内部错误通知（调用 error_callback_）
     *
     * @param error 错误描述文本
     */
    void notify_error(const std::string& error);

    /**
     * @brief 内部连接成功通知（调用 connection_callback_）
     */
    void notify_connection();

    /**
     * @brief 内部断开通知（调用 disconnection_callback_）
     */
    void notify_disconnection();

    /**
     * @brief 异步写队列项
     *
     * - binary: 标识此项是否为二进制数据
     * - text: 指向文本数据（若 binary=false）
     * - bin: 指向二进制数据（若 binary=true）
     *
     * 此结构使用 shared_ptr 保证在异步写期间数据有效。
     */
    struct Outgoing {
        bool binary;
        std::shared_ptr<std::string> text;
        std::shared_ptr<std::vector<uint8_t>> bin;
    };

    /**
     * @brief 启动或推进写操作（必须在 strand 上调用以保证序列化）
     *
     * 将从 write_queue_ 弹出下一项并触发异步写，直到队列为空或写在进行中。
     */
    void maybe_start_write();

    /**
     * @brief 执行底层关闭流程（释放资源并设置状态）
     *
     * 该函数在内部使用，负责安全地关闭 websocket 并停止 io 相关线程。
     */
    void do_close();

    boost::asio::io_context ioc_;
    boost::asio::ssl::context ssl_ctx_;

    boost::beast::flat_buffer buffer_;
    std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_plain_;
    std::unique_ptr<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>> ws_ssl_;

    // 保持 io_context 的工作对象，避免 run() 立即退出
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;

    // Strand 用来串行化对 websocket / 队列的访问（代替频繁锁）
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    // 写队列（仅在 strand 中访问）
    std::deque<Outgoing> write_queue_;
    bool write_in_progress_;

    std::atomic<bool> use_ssl_;
    std::atomic<bool> connected_;

    MessageCallback message_callback_;
    ErrorCallback error_callback_;
    ConnectionCallback connection_callback_;
    DisconnectionCallback disconnection_callback_;
    std::thread io_thread_;
    std::atomic<bool> should_stop_;

    // 标识 IO 线程是否实际在运行
    std::atomic<bool> io_thread_running_;
};

}
