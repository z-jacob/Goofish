#include "websocket_client.h"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace websocket_chat {

    WebSocketClient::WebSocketClient()
        : ssl_ctx_(ssl::context::tlsv12_client)
        , use_ssl_(false)
        , connected_(false)
        , should_stop_(false)
        , io_thread_running_(false) {
        // 不在构造里强制加载 paths，保留在 connect 中依据参数处理
    }

    WebSocketClient::~WebSocketClient() {
        disconnect();
    }

    bool WebSocketClient::connect(const std::string& host, const std::string& port, const std::string& target,
        bool verify_peer, const std::string& ca_file) {

        // 非阻塞：把实际的连接工作投递到 io_context 线程并立即返回（表示已排队）
        if (connected_) {
            notify_error("Already connected");
            return false;
        }

        // 确保 io_context 正在运行
        should_stop_ = false;
        // 如果之前 stop() 过，需要 restart()
        try {
            ioc_.restart();
        }
        catch (...) {
        }

        // 修复：使用 io_thread_running_ 来判断线程是否实际在运行
        if (!io_thread_running_.load(std::memory_order_acquire)) {
            // 如果线程对象存在但线程已退出，先 join 掉它（join 会立即返回）
            if (io_thread_.joinable()) {
                io_thread_.join();
            }
            // 启动新的 IO 线程，并在内部维护运行标志
            io_thread_ = std::thread([this]() {
                io_thread_running_.store(true, std::memory_order_release);
                // run() 返回时线程即将退出
                ioc_.run();
                io_thread_running_.store(false, std::memory_order_release);
            });
        }

        // 复制参数到 lambda 中
        std::string host_copy = host;
        std::string port_copy = port;
        std::string target_copy = target;
        bool verify_peer_copy = verify_peer;
        std::string ca_file_copy = ca_file;

        // 把连接逻辑放到 IO 线程执行，保持调用方非阻塞
        net::post(ioc_, [this, host_copy, port_copy, target_copy, verify_peer_copy, ca_file_copy]() {
            try {
                // 解析 host/target，支持带 scheme 的 host（例如 "wss://example.com"）
                std::string host_only = host_copy;
                std::string target_local = target_copy;
                bool detected_ssl = false;

                if (host_only.rfind("wss://", 0) == 0) {
                    detected_ssl = true;
                    host_only = host_only.substr(6);
                }
                else if (host_only.rfind("ws://", 0) == 0) {
                    detected_ssl = false;
                    host_only = host_only.substr(5);
                }
                else if (host_only.rfind("https://", 0) == 0) {
                    detected_ssl = true;
                    host_only = host_only.substr(8);
                }
                else if (host_only.rfind("http://", 0) == 0) {
                    detected_ssl = false;
                    host_only = host_only.substr(7);
                }

                auto pos = host_only.find('/');
                if (pos != std::string::npos) {
                    if (target_local.empty() || target_local == "/") {
                        target_local = host_only.substr(pos);
                    }
                    host_only = host_only.substr(0, pos);
                }

                if (port_copy == "443") {
                    detected_ssl = true;
                }

                use_ssl_ = detected_ssl;

                // SSL 配置：根据参数决定是否校验并加载 ca_file
                if (use_ssl_) {
                    // 重新初始化 ssl_ctx_ 为 client 模式（确保状态一致）
                    ssl_ctx_ = ssl::context(ssl::context::tlsv12_client);
                    if (!ca_file_copy.empty()) {
                        // 尝试加载传入的 CA bundle
                        boost::system::error_code ec;
                        ssl_ctx_.load_verify_file(ca_file_copy, ec);
                        if (ec) {
                            notify_error(std::string("Failed to load CA file: ") + ca_file_copy + " -> " + ec.message());
                            // 仍然继续，如果用户明确要求 verify_peer，则后续握手会失败并报告
                        }
                    }
                    else {
                        // 尝试默认路径（可能在某些平台不可用）
                        try {
                            ssl_ctx_.set_default_verify_paths();
                        }
                        catch (...) {
                            // 忽略，后续若需要校验可能失败
                        }
                    }

                    if (verify_peer_copy) {
                        ssl_ctx_.set_verify_mode(ssl::verify_peer);
                    }
                    else {
                        ssl_ctx_.set_verify_mode(ssl::verify_none);
                    }
                }

                // Host header 构造（带非标准端口）
                std::string host_header = host_only;
                if (!port_copy.empty() && port_copy != "80" && port_copy != "443") {
                    host_header += ":" + port_copy;
                }

                tcp::resolver resolver(ioc_);
                auto const results = resolver.resolve(host_only, port_copy);

                if (use_ssl_) {

                    ws_ssl_ = std::make_unique<websocket::stream<beast::ssl_stream<tcp::socket>>>(ioc_, ssl_ctx_);

                    // 设置 SNI（主机名）
                    if (!SSL_set_tlsext_host_name(ws_ssl_->next_layer().native_handle(), host_only.c_str())) {
                        notify_error("Failed to set SNI hostname");
                        return;
                    }

                    // 在 IO 线程执行阻塞 connect/handshake（安全，因为都在同一 IO 线程）
                    auto ep = net::connect(ws_ssl_->next_layer().next_layer(), results);

                    // SSL 握手（会触发证书校验/失败）
                    ws_ssl_->next_layer().handshake(ssl::stream_base::client);

                    ws_ssl_->set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
                    ws_ssl_->set_option(websocket::stream_base::decorator(
                        [](websocket::request_type& req) {
                            req.set(http::field::user_agent, "WebSocket-Chat-Client/1.0");
                        }));

                    ws_ssl_->handshake(host_header, target_local);

                }
                else {

                    ws_plain_ = std::make_unique<websocket::stream<tcp::socket>>(ioc_);

                    auto ep = net::connect(ws_plain_->next_layer(), results);

                    ws_plain_->set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
                    ws_plain_->set_option(websocket::stream_base::decorator(
                        [](websocket::request_type& req) {
                            req.set(http::field::user_agent, "WebSocket-Chat-Client/1.0");
                        }));

                    ws_plain_->handshake(host_header, target_local);
                }

                connected_ = true;
                notify_connection();

                // 启动异步读循环（必须在 IO 线程中）
                start_read_loop();
            }
            catch (const std::exception& e) {
                notify_error(std::string("Connection failed: ") + e.what());
            }
            });

        return true;
    }

    void WebSocketClient::start_read_loop() {
        if (!connected_) return;

        if (use_ssl_) {
            ws_ssl_->async_read(buffer_,
                [this](beast::error_code ec, std::size_t bytes) {
                    handle_message(ec, bytes);
                });
        }
        else {
            ws_plain_->async_read(buffer_,
                [this](beast::error_code ec, std::size_t bytes) {
                    handle_message(ec, bytes);
                });
        }
    }

    void WebSocketClient::handle_message(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            if (ec != websocket::error::closed && bytes_transferred > 0) {
                notify_error("Read error: " + ec.message());
            }
            connected_ = false;

            // 清理底层对象，避免重连时被旧的 socket/stream 干扰
            try {
                if (ws_plain_) {
                    beast::error_code ignore_ec;
                    ws_plain_->next_layer().close(ignore_ec);
                }
            } catch (...) {}
            try {
                if (ws_ssl_) {
                    beast::error_code ignore_ec;
                    ws_ssl_->next_layer().next_layer().close(ignore_ec);
                }
            } catch (...) {}

            ws_plain_.reset();
            ws_ssl_.reset();

            notify_disconnection();
            return;
        }

        if (message_callback_) {
            std::string message = beast::buffers_to_string(buffer_.data());
            bool is_binary = use_ssl_ ? ws_ssl_->got_binary() : ws_plain_->got_binary();
            message_callback_(message, is_binary);
        }

        buffer_.consume(bytes_transferred);
        start_read_loop();
    }

    bool WebSocketClient::send_text(const std::string& message) {
        if (!connected_) {
            notify_error("Not connected to server");
            return false;
        }

        // 将写操作投递到 IO 线程执行，立即返回（已排队）
        auto msg = std::make_shared<std::string>(message);
        net::post(ioc_, [this, msg]() {
            try {
                if (use_ssl_ && ws_ssl_) {
                    // 在 IO 线程上执行同步写，或改为 async_write 并提供 handler
                    ws_ssl_->write(net::buffer(*msg));
                }
                else if (ws_plain_) {
                    ws_plain_->write(net::buffer(*msg));
                }
            }
            catch (const std::exception& e) {
                notify_error(std::string("Send failed: ") + e.what());
            }
            });

        return true;
    }

    bool WebSocketClient::send_binary(const std::vector<uint8_t>& data) {
        if (!connected_) {
            notify_error("Not connected to server");
            return false;
        }

        // 将写操作投递到 IO 线程执行，立即返回（已排队）
        auto buf = std::make_shared<std::vector<uint8_t>>(data);
        net::post(ioc_, [this, buf]() {
            try {
                if (use_ssl_ && ws_ssl_) {
                    ws_ssl_->binary(true);
                    ws_ssl_->write(net::buffer(*buf));
                }
                else if (ws_plain_) {
                    ws_plain_->binary(true);
                    ws_plain_->write(net::buffer(*buf));
                }
            }
            catch (const std::exception& e) {
                notify_error(std::string("Binary send failed: ") + e.what());
            }
            });

        return true;
    }

    void WebSocketClient::disconnect() {
        if (!connected_) {
            // 仍然需要停止 io_context 并 join 线程以清理
            should_stop_ = true;
            ioc_.stop();
            if (io_thread_.joinable()) {
                io_thread_.join();
            }

            // 清理底层对象
            ws_plain_.reset();
            ws_ssl_.reset();

            notify_disconnection();

            return;
        }

        should_stop_ = true;
        connected_ = false;

        // 把关闭操作也投递到 IO 线程，确保对 stream 的访问发生在同一线程
        net::post(ioc_, [this]() {
            try {
                if (use_ssl_ && ws_ssl_) {
                    ws_ssl_->close(websocket::close_code::normal);
                }
                else if (ws_plain_) {
                    ws_plain_->close(websocket::close_code::normal);
                }
            }
            catch (const std::exception&) {
            }
            // 停止 io_context 将在调用端或这里进行
            ioc_.stop();
            });

        if (io_thread_.joinable()) {
            io_thread_.join();
        }

        // 清理底层对象
        ws_plain_.reset();
        ws_ssl_.reset();

        notify_disconnection();

    }

    void WebSocketClient::notify_error(const std::string& error) {
        if (error_callback_) {
            error_callback_(error);
        }
    }

    void WebSocketClient::notify_connection() {
        if (connection_callback_) {
            connection_callback_();
        }
    }

    void WebSocketClient::notify_disconnection()
    {
        if (disconnection_callback_) {
            disconnection_callback_();
        }
    }

}