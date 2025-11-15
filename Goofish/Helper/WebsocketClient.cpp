#include "WebsocketClient.h"

#include "Utils.h"


namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace websocket_chat {

	/**
	 * @brief 构造函数
	 *
	 * 初始化 SSL 上下文、io 工作守护、strand 以及原子状态变量。
	 * IO 线程尚未启动，实际运行在调用 connect 时触发。
	 */
	WebSocketClient::WebSocketClient()
		: ssl_ctx_(ssl::context::tlsv12_client)
		, use_ssl_(false)
		, connected_(false) {
	}

	/**
	 * @brief 析构函数
	 *
	 * 确保断开并清理所有 IO 相关资源。析构时会调用 disconnect()。
	 */
	WebSocketClient::~WebSocketClient() {
		Disconnect();
	}

	/**
	 * @brief 同步发起连接并完成 WebSocket 握手（在 strand 中执行实际逻辑）
	 *
	 * @param host  主机（可以包含协议前缀和路径）
	 * @param port  端口字符串（如 "80" 或 "443"）
	 * @param target WebSocket 路径
	 * @param verify_peer 是否校验证书链（仅用于 TLS）
	 * @param ca_file 可选 CA bundle 路径（为空使用系统默认）
	 * @return true 表示已成功调度连接过程（握手可能仍在执行）；false 表示立即失败
	 *
	 * 线程/同步语义：
	 * - 该函数会确保内部 io_context 正常运行并在单独线程中调用 ioc_.run()。
	 * - 实际的解析、socket 连接与握手在 strand 上执行以与其他异步操作序列化。
	 * - 连接成功后会设置 connected_ 并触发 connection_callback_，随后调用 start_read_loop()。
	 */
	bool WebSocketClient::Connect(const std::string& host, const std::string& port, const std::string& target,
		bool verify_peer, const std::string& ca_file) {

		if (connected_) {
			NotifyError(MODULE_INFO + "Already connected");
			return false;
		}

		try {
			std::string host_only = host;
			std::string target_local = target;
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

			if (port == "443") {
				detected_ssl = true;
			}

			use_ssl_ = detected_ssl;

			// SSL 配置
			if (use_ssl_) {
				ssl_ctx_ = ssl::context(ssl::context::tlsv12_client);
				if (!ca_file.empty()) {
					boost::system::error_code ec;
					ssl_ctx_.load_verify_file(ca_file, ec);
					if (ec) {
						NotifyError(MODULE_INFO + std::string("Failed to load CA file: ") + ca_file + " -> " + ec.message());
					}
				}
				else {
					ssl_ctx_.set_default_verify_paths();
				}
				ssl_ctx_.set_verify_mode(verify_peer ? ssl::verify_peer : ssl::verify_none);
			}

			std::string host_header = host_only;
			if (!port.empty() && port != "80" && port != "443") {
				host_header += ":" + port;
			}

			tcp::resolver resolver(ioc_);
			auto const results = resolver.resolve(host_only, port);

			if (use_ssl_) {
				ws_ssl_ = std::make_unique<websocket::stream<beast::ssl_stream<tcp::socket>>>(ioc_, ssl_ctx_);
				if (!SSL_set_tlsext_host_name(ws_ssl_->next_layer().native_handle(), host_only.c_str())) {
					NotifyError(MODULE_INFO + "Failed to set SNI hostname");
					return false;
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

			connected_ = true;
			NotifyConnection();
			return true;
		}
		catch (const std::exception& e) {
			NotifyError(MODULE_INFO + std::string("Connection failed: ") + e.what());
			connected_ = false;
			return false;
		}
	}

	/**
	 * @brief 将文本消息异步入队等待发送
	 *
	 * @param message UTF-8 文本
	 * @return true 表示已成功入队；false 表示当前未连接
	 *
	 * 语义：
	 * - 实际写操作由 write_queue_ 与 maybe_start_write() 管理；
	 * - 入队操作通过 post 到 strand 执行，线程安全。
	 */
	bool WebSocketClient::SendText(const std::string& message) {
		if (!connected_) {
			NotifyError(MODULE_INFO + "Not connected to server");
			return false;
		}
		try {
			if (use_ssl_ && ws_ssl_) {
				ws_ssl_->text(true);
				ws_ssl_->write(net::buffer(message));
			}
			else if (ws_plain_) {
				ws_plain_->text(true);
				ws_plain_->write(net::buffer(message));
			}
			return true;
		}
		catch (const std::exception& e) {
			NotifyError(MODULE_INFO + std::string("SendText exception: ") + e.what());
			return false;
		}
	}

	/**
	 * @brief 将二进制消息异步入队等待发送
	 *
	 * @param data 二进制数据
	 * @return true 表示已成功入队；false 表示当前未连接
	 *
	 * 行为同 send_text，但设置为二进制。
	 */
	bool WebSocketClient::SendBinary(const std::vector<uint8_t>& data) {
		if (!connected_) {
			NotifyError(MODULE_INFO + "Not connected to server");
			return false;
		}
		try {
			if (use_ssl_ && ws_ssl_) {
				ws_ssl_->binary(true);
				ws_ssl_->write(net::buffer(data));
			}
			else if (ws_plain_) {
				ws_plain_->binary(true);
				ws_plain_->write(net::buffer(data));
			}
			return true;
		}
		catch (const std::exception& e) {
			NotifyError(MODULE_INFO + std::string("SendBinary exception: ") + e.what());
			return false;
		}
	}

	/**
	 * @brief 主动断开连接并清理资源
	 *
	 * 语义：
	 * - 将断开逻辑 post 到 strand 中执行以保证线程安全；
	 * - 会关闭底层 websocket，清空写队列，重置 work_guard_ 以结束 io_context::run；
	 * - 等待 io 线程退出后返回。
	 */
	void WebSocketClient::Disconnect() {
		if (!connected_) return;
		try {
			if (use_ssl_ && ws_ssl_) {
				beast::error_code ec;
				ws_ssl_->close(websocket::close_code::normal, ec);
			}
			else if (ws_plain_) {
				beast::error_code ec;
				ws_plain_->close(websocket::close_code::normal, ec);
			}
		}
		catch (...) {}
		ws_plain_.reset();
		ws_ssl_.reset();
		connected_ = false;
		NotifyDisconnection();
	}

	/**
	 * @brief 错误通知适配器
	 *
	 * 直接调用用户提供的 error_callback_（回调在 IO 线程执行）。
	 * 如果需要在 UI 线程处理，应由回调实现方负责线程切换。
	 */
	void WebSocketClient::NotifyError(const std::string& error) {
		// 回调直接调用（通常在 IO 线程）。如果需要在 UI 线程调用，外部应在回调中切换。
		if (error_callback_) {
			error_callback_(error);
		}
	}

	/**
	 * @brief 连接成功通知适配器
	 *
	 * 直接调用 connection_callback_（回调在 IO 线程执行）。
	 */
	void WebSocketClient::NotifyConnection() {
		if (connection_callback_) {
			connection_callback_();
		}
	}

	/**
	 * @brief 断开连接通知适配器
	 *
	 * 直接调用 disconnection_callback_（回调在 IO 线程执行）。
	 */
	void WebSocketClient::NotifyDisconnection() {
		if (disconnection_callback_) {
			disconnection_callback_();
		}
	}

	void WebSocketClient::NotifyMessage(const std::string& message, bool is_binary)
	{
		if (message_callback_) {
			message_callback_(message, is_binary);
		}
	}

	bool WebSocketClient::Receive() {
		if (!connected_) return false;
		try {
			std::string out_message;
			bool is_binary;

			beast::flat_buffer buffer;
			if (use_ssl_ && ws_ssl_) {
				ws_ssl_->read(buffer);
				is_binary = ws_ssl_->got_binary();
			}
			else if (ws_plain_) {
				ws_plain_->read(buffer);
				is_binary = ws_plain_->got_binary();
			}
			out_message = beast::buffers_to_string(buffer.data());
			NotifyMessage(out_message, is_binary);
			return true;
		}
		catch (const beast::system_error& e) {
			// 检查是否为 EOF（服务器断开）
			if (e.code() == net::error::eof) {
				NotifyError(MODULE_INFO +
					"Server closed connection (EOF)");
				ws_plain_.reset();
				ws_ssl_.reset();
				connected_ = false;
				NotifyDisconnection();
			}
			else {
				NotifyError(MODULE_INFO + std::string("Receive exception: ") + e.what());
				connected_ = false;
			}
			return false;
		}
		catch (const std::exception& e) {
			NotifyError(MODULE_INFO + std::string("Receive exception: ") + e.what());
			ws_plain_.reset();
			ws_ssl_.reset();
			connected_ = false;
			return false;
		}
	}

}