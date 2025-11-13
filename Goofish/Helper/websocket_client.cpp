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
    , should_stop_(false) {
    // 不在构造里强制加载 paths，保留在 connect 中依据参数处理
}

WebSocketClient::~WebSocketClient() {
    disconnect();
}

bool WebSocketClient::connect(const std::string& host, const std::string& port, const std::string& target,
                              bool verify_peer, const std::string& ca_file) {
    try {
        // 解析 host/target，支持带 scheme 的 host（例如 "wss://example.com"）
        std::string host_only = host;
        std::string target_local = target;
        bool detected_ssl = false;

        if (host_only.rfind("wss://", 0) == 0) {
            detected_ssl = true;
            host_only = host_only.substr(6);
        } else if (host_only.rfind("ws://", 0) == 0) {
            detected_ssl = false;
            host_only = host_only.substr(5);
        } else if (host_only.rfind("https://", 0) == 0) {
            detected_ssl = true;
            host_only = host_only.substr(8);
        } else if (host_only.rfind("http://", 0) == 0) {
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

        if (port == "443") {
            detected_ssl = true;
        }

        use_ssl_ = detected_ssl;

        // SSL 配置：根据参数决定是否校验并加载 ca_file
        if (use_ssl_) {
            // 重新初始化 ssl_ctx_ 为 client 模式（确保状态一致）
            ssl_ctx_ = ssl::context(ssl::context::tlsv12_client);
            if (!ca_file.empty()) {
                // 尝试加载传入的 CA bundle
                boost::system::error_code ec;
                ssl_ctx_.load_verify_file(ca_file, ec);
                if (ec) {
                    notify_error(std::string("Failed to load CA file: ") + ca_file + " -> " + ec.message());
                    // 仍然继续，如果用户明确要求 verify_peer，则后续握手会失败并报告
                }
            } else {
                // 尝试默认路径（可能在某些平台不可用）
                try {
                    ssl_ctx_.set_default_verify_paths();
                } catch (...) {
                    // 忽略，后续若需要校验可能失败
                }
            }

            if (verify_peer) {
                ssl_ctx_.set_verify_mode(ssl::verify_peer);
            } else {
                ssl_ctx_.set_verify_mode(ssl::verify_none);
            }
        }

        // Host header 构造（带非标准端口）
        std::string host_header = host_only;
        if (!port.empty() && port != "80" && port != "443") {
            host_header += ":" + port;
        }

        tcp::resolver resolver(ioc_);
        auto const results = resolver.resolve(host_only, port);

        if (use_ssl_) {
            
            ws_ssl_ = std::make_unique<websocket::stream<beast::ssl_stream<tcp::socket>>>(ioc_, ssl_ctx_);
            
            // 设置 SNI（主机名）
            if (!SSL_set_tlsext_host_name(ws_ssl_->next_layer().native_handle(), host_only.c_str())) {
                notify_error("Failed to set SNI hostname");
                return false;
            }
            
            auto ep = net::connect(ws_ssl_->next_layer().next_layer(), results);
            
            // SSL 握手（会触发证书校验/失败）
            ws_ssl_->next_layer().handshake(ssl::stream_base::client);
            
            ws_ssl_->set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
            ws_ssl_->set_option(websocket::stream_base::decorator(
                [](websocket::request_type& req) {
                    req.set(http::field::user_agent, "WebSocket-Chat-Client/1.0");
                }));
            
            ws_ssl_->handshake(host_header, target_local);
            
        } else {
            
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
        notify_connection_status(true);
        
        should_stop_ = false;
        io_thread_ = std::thread([this]() {
            ioc_.run();
        });
        
        return true;
        
    } catch (const std::exception& e) {
        notify_error(std::string("Connection failed: ") + e.what());
        return false;
    }
}

void WebSocketClient::start_read_loop() {
    if (!connected_) return;
    
    if (use_ssl_) {
        ws_ssl_->async_read(buffer_, 
            [this](beast::error_code ec, std::size_t bytes) {
                handle_message(ec, bytes);
            });
    } else {
        ws_plain_->async_read(buffer_, 
            [this](beast::error_code ec, std::size_t bytes) {
                handle_message(ec, bytes);
            });
    }
}

void WebSocketClient::handle_message(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        if (ec != websocket::error::closed) {
            notify_error("Read error: " + ec.message());
        }
        connected_ = false;
        notify_connection_status(false);
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
    
    try {
        if (use_ssl_) {
            ws_ssl_->write(net::buffer(message));
        } else {
            ws_plain_->write(net::buffer(message));
        }
        return true;
    } catch (const std::exception& e) {
        notify_error("Send failed: " + std::string(e.what()));
        return false;
    }
}

bool WebSocketClient::send_binary(const std::vector<uint8_t>& data) {
    if (!connected_) {
        notify_error("Not connected to server");
        return false;
    }
    
    try {
        if (use_ssl_) {
            ws_ssl_->binary(true);
            ws_ssl_->write(net::buffer(data));
        } else {
            ws_plain_->binary(true);
            ws_plain_->write(net::buffer(data));
        }
        return true;
    } catch (const std::exception& e) {
        notify_error("Binary send failed: " + std::string(e.what()));
        return false;
    }
}

void WebSocketClient::disconnect() {
    if (!connected_) return;
    
    should_stop_ = true;
    connected_ = false;
    
    try {
        if (use_ssl_ && ws_ssl_) {
            ws_ssl_->close(websocket::close_code::normal);
        } else if (ws_plain_) {
            ws_plain_->close(websocket::close_code::normal);
        }
    } catch (const std::exception& e) {    
    }
    
    ioc_.stop();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
    
    notify_connection_status(false);
}

void WebSocketClient::notify_error(const std::string& error) {
    if (error_callback_) {
        error_callback_(error);
    }
}

void WebSocketClient::notify_connection_status(bool connected) {
    if (connection_callback_) {
        connection_callback_(connected);
    }
}

}