#include "WebsocketClientSystem.h"
#include <regex>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <optional>
#include "../Helper/WebsocketEvents.h"

/**
 * @file WebsocketClientSystem.cpp
 * @brief 将底层 websocket_chat::WebSocketClient 适配到系统框架（JFramework）
 *
 * 该实现负责：
 * - 提供按 host/port/target/use_ssl 的连接接口（向后兼容）；
 * - 提供按单一 path/url 的便捷连接接口（自动解析 scheme/host/port/target）；
 * - 提供发送与关闭操作的封装并通过事件系统上报连接/断开/错误/收到消息。
 *
 * 已优化点（面向对象设计原则）：
 * - 将 URL 解析职责抽离到本地函数（单一职责），简化成员方法逻辑；
 * - 统一错误事件发送逻辑，减少重复代码；
 * - 减少对底层实现内部行为的假设（移除对 start_read_loop 的冗余调用）；
 * - 保持异常安全并清晰地向上层报告错误。
 */

namespace {
	struct ParsedUrl {
		std::string host;
		unsigned short port;
		std::string target;
		bool use_ssl;
	};

	// 修剪字符串两端空白
	inline std::string trim_copy(const std::string& s) {
		auto l = s.find_first_not_of(" \t\n\r");
		if (l == std::string::npos) return {};
		auto r = s.find_last_not_of(" \t\n\r");
		return s.substr(l, r - l + 1);
	}

	// 解析 websocket URL/path，成功返回 ParsedUrl，否则 return std::nullopt
	std::optional<ParsedUrl> parse_ws_url(const std::string& path) {
		std::string s = trim_copy(path);
		if (s.empty()) return std::nullopt;

		// 正则提取：可选 scheme、主机、可选端口、可选路径
		std::regex re(R"(^(?:(ws|wss)://)?([^/:]+)(?::(\d+))?(/.*)?$)", std::regex::icase);
		std::smatch m;
		if (!std::regex_match(s, m, re)) {
			return std::nullopt;
		}

		std::string scheme = m[1].matched ? m[1].str() : "";
		std::string host = m[2].matched ? m[2].str() : "";
		std::string port_str = m[3].matched ? m[3].str() : "";
		std::string target = m[4].matched ? m[4].str() : "/";

		bool use_ssl = false;
		if (!scheme.empty()) {
			std::string scheme_l;
			scheme_l.reserve(scheme.size());
			std::transform(scheme.begin(), scheme.end(), std::back_inserter(scheme_l), [](unsigned char ch) { return std::tolower(ch); });
			use_ssl = (scheme_l == "wss");
		}

		unsigned short port = 0;
		if (port_str.empty()) {
			port = use_ssl ? 443 : 80;
		}
		else {
			long p = std::strtol(port_str.c_str(), nullptr, 10);
			if (p <= 0 || p > 65535) return std::nullopt;
			port = static_cast<unsigned short>(p);
		}

		if (target.empty()) target = "/";

		return ParsedUrl{ host, port, target, use_ssl };
	}
}

bool WebsocketClientSystem::Connect(const std::string& host, unsigned short port, const std::string& target, bool use_ssl)
{
	const std::string port_str = std::to_string(port);
	const std::string ca_file = "cacert.pem";

	// 统一错误事件发送简易 lambda
	auto sendErr = [this](const std::string& msg) {
		this->SendEvent<WebsocketErrorEvent>(msg);
		};

	try {
		// 调用底层 client.connect（connect 会在内部启动 io 线程并在握手成功后触发回调）
		if (!client.connect(host, port_str, target, use_ssl, ca_file)) {
			sendErr("[WebsocketClientSystem] Connect failed: Unable to connect to " + host + ":" + std::to_string(port));
			return false;
		}

		// 之前代码显式调用 client.start_read_loop()，但底层在握手成功后会启动读循环。
		// 这里移除冗余调用以避免对底层实现的假设。

		return true;
	}
	catch (const std::exception& ex) {
		sendErr(std::string("[WebsocketClientSystem] Connect exception: ") + ex.what());
		return false;
	}
}

bool WebsocketClientSystem::Connect(const std::string& path)
{
	auto sendErr = [this](const std::string& msg) {
		this->SendEvent<WebsocketErrorEvent>(msg);
		};

	try {
		auto parsed = parse_ws_url(path);
		if (!parsed) {
			sendErr(std::string("[WebsocketClientSystem] Connect(path) parse failed: invalid format: ") + path);
			return false;
		}

		return Connect(parsed->host, parsed->port, parsed->target, parsed->use_ssl);
	}
	catch (const std::exception& ex) {
		sendErr(std::string("[WebsocketClientSystem] Connect(path) exception: ") + ex.what());
		return false;
	}
}

bool WebsocketClientSystem::Send(const std::string& msg)
{
	auto sendErr = [this](const std::string& msg) {
		this->SendEvent<WebsocketErrorEvent>(msg);
		};

	if (!client.is_connected()) {
		sendErr("[WebsocketClientSystem] Send failed: not connected");
		return false;
	}
	try {
		if (!client.send_text(msg)) {
			sendErr("[WebsocketClientSystem] send_text failed");
			return false;
		}
		return true;
	}
	catch (const std::exception& ex) {
		sendErr(std::string("[WebsocketClientSystem] Send exception: ") + ex.what());
		return false;
	}
}

void WebsocketClientSystem::Close()
{
	auto sendErr = [this](const std::string& msg) {
		this->SendEvent<WebsocketErrorEvent>(msg);
		};

	try {
		client.disconnect();
	}
	catch (const std::exception& ex) {
		sendErr(std::string("[WebsocketClientSystem] Close exception: ") + ex.what());
	}
}

void WebsocketClientSystem::OnInit()
{
	auto sendErr = [this](const std::string& msg) {
		this->SendEvent<WebsocketErrorEvent>(msg);
		};

	try {
		// 将底层回调映射为框架事件
		client.set_connection_callback([this]() {
			this->SendEvent<WebsocketConnectionEvent>();
			});

		client.set_disconnection_callback([this]() {
			this->SendEvent<WebsocketDisconnectionEvent>();
			});

		client.set_error_callback([this](const std::string& err) {
			this->SendEvent<WebsocketErrorEvent>(err);
			});

		client.set_message_callback([this](const std::string& msg, bool is_binary) {
			this->SendEvent<WebsocketReceiveEvent>(msg, is_binary);
			});
	}
	catch (const std::exception& ex) {
		sendErr(std::string("[WebsocketClientSystem] OnInit exception: ") + ex.what());
	}
}

void WebsocketClientSystem::OnDeinit()
{
	Close();
}

void WebsocketClientSystem::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{
	// 如果需要把事件系统与 websocket 交互，请在此实现
}
