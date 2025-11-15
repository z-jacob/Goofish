#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

namespace websocket_chat {

class WebSocketClient {
public:
    using MessageCallback = std::function<void(const std::string&, bool)>; 
    using ErrorCallback = std::function<void(const std::string&)>;
    using ConnectionCallback = std::function<void()>;
    using DisconnectionCallback = std::function<void()>;

    WebSocketClient();
    ~WebSocketClient();
    // 新增可选参数：verify_peer (是否校验证书)，ca_file (CA bundle 的路径)
    bool connect(const std::string& host, const std::string& port, const std::string& target,
                 bool verify_peer = true, const std::string& ca_file = "");
    
    bool send_text(const std::string& message);
    bool send_binary(const std::vector<uint8_t>& data);
    void disconnect();
    bool is_connected() const { return connected_; }
    
    void set_message_callback(MessageCallback callback) { message_callback_ = std::move(callback); }
    void set_error_callback(ErrorCallback callback) { error_callback_ = std::move(callback); }
    void set_connection_callback(ConnectionCallback callback) { connection_callback_ = std::move(callback); }
    void set_disconnection_callback(DisconnectionCallback callback) { disconnection_callback_ = std::move(callback); }
    
    void start_read_loop();

private:
    void handle_message(boost::beast::error_code ec, std::size_t bytes_transferred);
    void notify_error(const std::string& error);
    void notify_connection();
    void notify_disconnection();

    boost::asio::io_context ioc_;
    boost::asio::ssl::context ssl_ctx_;
    std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_plain_;
    std::unique_ptr<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>> ws_ssl_;
    
    boost::beast::flat_buffer buffer_;
    bool use_ssl_;
    bool connected_;
    
    MessageCallback message_callback_;
    ErrorCallback error_callback_;
    ConnectionCallback connection_callback_;
    DisconnectionCallback disconnection_callback_;
    std::thread io_thread_;
    std::atomic<bool> should_stop_;

    // 新增：标识 IO 线程是否实际在运行（防止 thread 对象已存在但线程已退出未 join 导致误判）
    std::atomic<bool> io_thread_running_;
};

}
