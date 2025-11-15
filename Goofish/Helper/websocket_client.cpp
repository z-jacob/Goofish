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
        , work_guard_(net::make_work_guard(ioc_))
        , strand_(ioc_.get_executor())
        , write_in_progress_(false)
        , use_ssl_(false)
        , connected_(false)
        , should_stop_(false)
        , io_thread_running_(false) {
    }

    WebSocketClient::~WebSocketClient() {
        disconnect();
    }

    bool WebSocketClient::connect(const std::string& host, const std::string& port, const std::string& target,
        bool verify_peer, const std::string& ca_file) {

        if (connected_.load(std::memory_order_acquire)) {
            notify_error("Already connected");
            return false;
        }

        should_stop_.store(false, std::memory_order_release);

        // restart io_context if needed and ensure thread is running
        ioc_.restart();
        if (!io_thread_running_.load(std::memory_order_acquire)) {
            if (io_thread_.joinable()) {
                io_thread_.join();
            }
            io_thread_ = std::thread([this]() {
                io_thread_running_.store(true, std::memory_order_release);
                // run will block until stop is called or work_guard_ is reset
                ioc_.run();
                io_thread_running_.store(false, std::memory_order_release);
            });
        }

        // copy parameters for lambda
        std::string host_copy = host;
        std::string port_copy = port;
        std::string target_copy = target;
        bool verify_peer_copy = verify_peer;
        std::string ca_file_copy = ca_file;

        // run connection logic in strand (serialized with other operations)
        net::post(strand_, [this, host_copy, port_copy, target_copy, verify_peer_copy, ca_file_copy]() {
            try {
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

                use_ssl_.store(detected_ssl, std::memory_order_release);

                // SSL 配置
                if (use_ssl_) {
                    ssl_ctx_ = ssl::context(ssl::context::tlsv12_client);
                    if (!ca_file_copy.empty()) {
                        boost::system::error_code ec;
                        ssl_ctx_.load_verify_file(ca_file_copy, ec);
                        if (ec) {
                            notify_error(std::string("Failed to load CA file: ") + ca_file_copy + " -> " + ec.message());
                        }
                    }
                    else {
                        try {
                            ssl_ctx_.set_default_verify_paths();
                        }
                        catch (...) {}
                    }

                    if (verify_peer_copy) {
                        ssl_ctx_.set_verify_mode(ssl::verify_peer);
                    }
                    else {
                        ssl_ctx_.set_verify_mode(ssl::verify_none);
                    }
                }

                std::string host_header = host_only;
                if (!port_copy.empty() && port_copy != "80" && port_copy != "443") {
                    host_header += ":" + port_copy;
                }

                tcp::resolver resolver(ioc_);
                auto const results = resolver.resolve(host_only, port_copy);

                if (use_ssl_) {
                    ws_ssl_ = std::make_unique<websocket::stream<beast::ssl_stream<tcp::socket>>>(ioc_, ssl_ctx_);

                    if (!SSL_set_tlsext_host_name(ws_ssl_->next_layer().native_handle(), host_only.c_str())) {
                        notify_error("Failed to set SNI hostname");
                        return;
                    }

                    net::connect(ws_ssl_->next_layer().next_layer(), results);
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

                    net::connect(ws_plain_->next_layer(), results);

                    ws_plain_->set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
                    ws_plain_->set_option(websocket::stream_base::decorator(
                        [](websocket::request_type& req) {
                            req.set(http::field::user_agent, "WebSocket-Chat-Client/1.0");
                        }));

                    ws_plain_->handshake(host_header, target_local);
                }

                connected_.store(true, std::memory_order_release);
                notify_connection();

                // 启动异步读循环
                start_read_loop();
            }
            catch (const std::exception& e) {
                notify_error(std::string("Connection failed: ") + e.what());
            }
        });

        return true;
    }

    void WebSocketClient::start_read_loop() {
        if (!connected_.load(std::memory_order_acquire)) return;

        // 继续在 strand 中执行读回调的排队，保证与写互不干扰
        if (use_ssl_.load(std::memory_order_acquire)) {
            ws_ssl_->async_read(buffer_,
                net::bind_executor(strand_, [this](beast::error_code ec, std::size_t bytes) {
                    handle_message(ec, bytes);
                }));
        }
        else {
            ws_plain_->async_read(buffer_,
                net::bind_executor(strand_, [this](beast::error_code ec, std::size_t bytes) {
                    handle_message(ec, bytes);
                }));
        }
    }

    void WebSocketClient::handle_message(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            if (ec != websocket::error::closed) {
                notify_error(std::string("Read error: ") + ec.message());
            }

            connected_.store(false, std::memory_order_release);

            // 关闭并清理（在 strand 上执行）
            ws_plain_.reset();
            ws_ssl_.reset();

            notify_disconnection();
            return;
        }

        if (message_callback_) {
            std::string message = beast::buffers_to_string(buffer_.data());
            bool is_binary = use_ssl_.load(std::memory_order_acquire) ? (ws_ssl_ ? ws_ssl_->got_binary() : false)
                                                                       : (ws_plain_ ? ws_plain_->got_binary() : false);
            // 回调可以运行在 IO 线程（如果需要在外部线程运行，应用层可在回调内自己移交）
            message_callback_(message, is_binary);
        }

        // 消耗已读数据
        buffer_.consume(bytes_transferred);

        // 继续读取
        start_read_loop();
    }

    // 向写队列入队并在必要时启动异步写（post 到 strand）
    bool WebSocketClient::send_text(const std::string& message) {
        if (!connected_.load(std::memory_order_acquire)) {
            notify_error("Not connected to server");
            return false;
        }

        auto msg = std::make_shared<std::string>(message);
        net::post(strand_, [this, msg]() {
            write_queue_.push_back(Outgoing{ false, msg, nullptr });
            maybe_start_write();
        });

        return true;
    }

    bool WebSocketClient::send_binary(const std::vector<uint8_t>& data) {
        if (!connected_.load(std::memory_order_acquire)) {
            notify_error("Not connected to server");
            return false;
        }

        auto buf = std::make_shared<std::vector<uint8_t>>(data);
        net::post(strand_, [this, buf]() {
            write_queue_.push_back(Outgoing{ true, nullptr, buf });
            maybe_start_write();
        });

        return true;
    }

    // 必须在 strand 中调用
    void WebSocketClient::maybe_start_write() {
        if (write_in_progress_) return;
        if (write_queue_.empty()) return;
        write_in_progress_ = true;

        Outgoing out = std::move(write_queue_.front());
        write_queue_.pop_front();

        try {
            if (out.binary) {
                if (use_ssl_.load(std::memory_order_acquire) && ws_ssl_) {
                    ws_ssl_->binary(true);
                    ws_ssl_->async_write(net::buffer(*out.bin),
                        net::bind_executor(strand_, [this](beast::error_code ec, std::size_t /*bytes*/) {
                            if (ec) {
                                notify_error(std::string("Write error: ") + ec.message());
                                // 出错时关闭连接
                                connected_.store(false, std::memory_order_release);
                                ws_plain_.reset();
                                ws_ssl_.reset();
                                notify_disconnection();
                                write_in_progress_ = false;
                                return;
                            }
                            // 继续下一个写
                            write_in_progress_ = false;
                            maybe_start_write();
                        }));
                }
                else if (ws_plain_) {
                    ws_plain_->binary(true);
                    ws_plain_->async_write(net::buffer(*out.bin),
                        net::bind_executor(strand_, [this](beast::error_code ec, std::size_t /*bytes*/) {
                            if (ec) {
                                notify_error(std::string("Write error: ") + ec.message());
                                connected_.store(false, std::memory_order_release);
                                ws_plain_.reset();
                                ws_ssl_.reset();
                                notify_disconnection();
                                write_in_progress_ = false;
                                return;
                            }
                            write_in_progress_ = false;
                            maybe_start_write();
                        }));
                }
                else {
                    // 没有可用 socket，丢弃并报告错误
                    notify_error("Write failed: no websocket available");
                    write_in_progress_ = false;
                    maybe_start_write();
                }
            }
            else {
                if (use_ssl_.load(std::memory_order_acquire) && ws_ssl_) {
                    ws_ssl_->text(true);
                    ws_ssl_->async_write(net::buffer(*out.text),
                        net::bind_executor(strand_, [this](beast::error_code ec, std::size_t /*bytes*/) {
                            if (ec) {
                                notify_error(std::string("Write error: ") + ec.message());
                                connected_.store(false, std::memory_order_release);
                                ws_plain_.reset();
                                ws_ssl_.reset();
                                notify_disconnection();
                                write_in_progress_ = false;
                                return;
                            }
                            write_in_progress_ = false;
                            maybe_start_write();
                        }));
                }
                else if (ws_plain_) {
                    ws_plain_->text(true);
                    ws_plain_->async_write(net::buffer(*out.text),
                        net::bind_executor(strand_, [this](beast::error_code ec, std::size_t /*bytes*/) {
                            if (ec) {
                                notify_error(std::string("Write error: ") + ec.message());
                                connected_.store(false, std::memory_order_release);
                                ws_plain_.reset();
                                ws_ssl_.reset();
                                notify_disconnection();
                                write_in_progress_ = false;
                                return;
                            }
                            write_in_progress_ = false;
                            maybe_start_write();
                        }));
                }
                else {
                    notify_error("Write failed: no websocket available");
                    write_in_progress_ = false;
                    maybe_start_write();
                }
            }
        }
        catch (const std::exception& e) {
            notify_error(std::string("Send exception: ") + e.what());
            write_in_progress_ = false;
            // 继续尝试下一个
            maybe_start_write();
        }
    }

    void WebSocketClient::disconnect() {
        // 标记停止并在 strand 中安全关闭
        should_stop_.store(true, std::memory_order_release);

        // 如果之前没有启动过 io 线程，也需要清理资源
        net::post(strand_, [this]() {
            if (!connected_.load(std::memory_order_acquire)) {
                // 即使未连接，也要释放 work guard 让 run() 退出
                work_guard_.reset();
                return;
            }

            connected_.store(false, std::memory_order_release);

            try {
                if (use_ssl_.load(std::memory_order_acquire) && ws_ssl_) {
                    beast::error_code ec;
                    ws_ssl_->close(websocket::close_code::normal, ec);
                    (void)ec;
                }
                else if (ws_plain_) {
                    beast::error_code ec;
                    ws_plain_->close(websocket::close_code::normal, ec);
                    (void)ec;
                }
            }
            catch (...) {}

            ws_plain_.reset();
            ws_ssl_.reset();

            // 清空写队列
            write_queue_.clear();
            write_in_progress_ = false;

            // 释放 work guard，允许 run() 退出
            work_guard_.reset();

            notify_disconnection();
        });

        // 等待 io 线程退出
        if (io_thread_.joinable()) {
            io_thread_.join();
        }

        // 最终确保状态
        connected_.store(false, std::memory_order_release);
    }

    void WebSocketClient::notify_error(const std::string& error) {
        // 回调直接调用（通常在 IO 线程）。如果需要在 UI 线程调用，外部应在回调中切换。
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