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

namespace websocket_chat {

class WebSocketClient {
public:
    using MessageCallback = std::function<void(const std::string&, bool)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using ConnectionCallback = std::function<void()>;
    using DisconnectionCallback = std::function<void()>;

    WebSocketClient();
    ~WebSocketClient();

    // 同步连接
    bool Connect(const std::string& host, const std::string& port, const std::string& target,
                 bool verify_peer = true, const std::string& ca_file = "");

    // 同步发送文本
    bool SendText(const std::string& message);

    // 同步发送二进制
    bool SendBinary(const std::vector<uint8_t>& data);

    // 同步断开
    void Disconnect();

    bool IsConnected() const { return connected_; }

    bool Receive();

    // 回调接口（可选，连接/断开/错误时通知）
    void SetMessageCallback(MessageCallback callback) { message_callback_ = std::move(callback); }
    void SetErrorCallback(ErrorCallback callback) { error_callback_ = std::move(callback); }
    void SetConnectionCallback(ConnectionCallback callback) { connection_callback_ = std::move(callback); }
    void SetDisconnectionCallback(DisconnectionCallback callback) { disconnection_callback_ = std::move(callback); }

private:
    boost::asio::io_context ioc_;
    boost::asio::ssl::context ssl_ctx_;
    boost::beast::flat_buffer buffer_;
    std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_plain_;
    std::unique_ptr<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>> ws_ssl_;
    bool use_ssl_;
    bool connected_;

    MessageCallback message_callback_;
    ErrorCallback error_callback_;
    ConnectionCallback connection_callback_;
    DisconnectionCallback disconnection_callback_;

    void NotifyConnection();
    void NotifyDisconnection();
    void NotifyMessage(const std::string& message, bool is_binary);
};

}
