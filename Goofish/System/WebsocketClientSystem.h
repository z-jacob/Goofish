#pragma once
#include "../Helper/JFramework.h"
#include "../Helper/WebsocketClient.h"

#include <memory>
#include <string>
#include <atomic>

/**
 * @brief 系统层的 WebSocket 客户端适配器
 *
 * 将底层的 websocket_chat::WebSocketClient 封装为 JFramework::AbstractSystem 的一员，
 * 提供便捷的连接、发送与关闭接口，并在系统生命周期回调（OnInit/OnDeinit/OnEvent）中
 * 可接入事件处理与初始化逻辑。
 *
 * 线程与回调语义：
 * - 底层 WebSocketClient 的回调（消息/错误/连接/断开）通常在 IO 线程执行，若需要在主线程处理，
 *   请在回调中做线程切换或在本类的事件派发中转发到主线程。
 * - Connect/Send/Close 可以在任意线程调用；底层 client 已在内部使用 strand 序列化 IO 操作。
 */
class WebsocketClientSystem : public JFramework::AbstractSystem
{
public:
    // 可通过构造函数注入 CA 文件路径，便于测试和配置
    explicit WebsocketClientSystem(const std::string& ca_file = "cacert.pem") noexcept;
    virtual ~WebsocketClientSystem() noexcept override = default;

    /**
     * @brief 解析并连接到指定 path/url
     *
     * 支持的输入示例：
     * - "ws://example.com:1234/some/path"
     * - "wss://example.com/some/path"
     * - "example.com:1234/path"
     * - "example.com"（默认端口和 path 需根据实现约定）
     *
     * 函数会从 path 中解析 host、port、target 以及是否使用 SSL（wss/https 或 443 端口）
     * 并调用底层 Connect。
     *
     * @param path 待解析的地址或完整 URL
     * @return true 成功启动连接流程（不代表握手完成），false 表示立即失败
     */
    bool Connect(const std::string& path);

    /**
     * @brief 发送文本消息到已连接的 WebSocket 服务器
     *
     * 该方法会将消息交由底层客户端异步发送。若当前未连接会返回 false 并触发错误回调。
     *
     * @param msg 要发送的 UTF-8 文本
     * @return true 表示已入队待发送，false 表示未连接或发送失败
     */
    bool Send(const std::string& msg);

    /**
     * @brief 主动关闭与服务器的连接
     *
     * 触发关闭流程并清理底层资源，OnDeinit/断开回调将按照系统与客户端实现被调用。
     */
    void Close();

    /**
     * @brief 获取当前连接状态
     *
     * @return true 已连接，false 未连接
     */
    bool IsConnected();

protected:
    /**
     * @brief 系统初始化回调
     *
     * 当系统被 JFramework 启动时调用。可在此处初始化 client 回调、注册事件监听等。
     */
    void OnInit() override;

    /**
     * @brief 系统反初始化回调
     *
     * 当系统被 JFramework 停止时调用。应在此处确保 client 已关闭并释放资源。
     */
    void OnDeinit() override;

    /**
     * @brief 系统事件分发回调
     *
     * 接收并处理来自框架的事件（如来自 UI/网络/其他系统的消息），
     * 可根据事件内容调用 Connect/Send/Close 等方法。
     *
     * @param event 框架传来的事件对象（可能为 nullptr，请按实现检查）
     */
    void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;

    // 兼容旧接口：按 host/port/target/use_ssl 连接
    /**
     * @brief 通过分离的 host/port/target 参数连接（向后兼容）
     *
     * @param host 服务器主机名或 IP（不包含协议前缀）
     * @param port TCP 端口号
     * @param target WebSocket 路径，默认为 "/"
     * @param use_ssl 是否使用 TLS（wss），默认 true
     * @return true 表示已调度连接流程，false 表示立即失败
     */
    bool Connect(const std::string& host, unsigned short port, bool use_ssl = true);

private:
    // CA 文件路径，可通过构造函数配置（便于测试/运行时替换）
    std::string ca_file_;

    // 标记系统是否处于已初始化状态（用于避免在已反初始化后继续派发事件）
    std::atomic<bool> initialized_{ false };

    // 底层实际的 websocket 客户端实例
    WebSocketClient client;
};

