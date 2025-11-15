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
 * 本次优化关注点（面向对象原则）：
 * - 引入依赖注入（CA 文件路径通过构造函数注入，便于测试与配置）；
 * - 将重复的错误上报抽象为成员函数 send_error（封装，开闭）；
 * - 增加 initialized_ 原子标记，回调在派发事件前检查，避免反初始化期间或之后派发事件；
 * - 保持 URL 解析职责独立（匿名命名空间内函数），保持 Connect 系列方法简洁。
 */

namespace {
	struct ParsedUrl {
		std::string host;
		unsigned short port;
		std::string target;
		bool use_ssl;
	};

	// 修剪字符串两端空白
	inline std::string TrimCopy(const std::string& s) {
		auto l = s.find_first_not_of(" \t\n\r");
		if (l == std::string::npos) return {};
		auto r = s.find_last_not_of(" \t\n\r");
		return s.substr(l, r - l + 1);
	}

	// 解析 websocket URL/path，成功返回 ParsedUrl，否则 return std::nullopt
	std::optional<ParsedUrl> ParseWebsocketUrl(const std::string& path) {
		std::string s = TrimCopy(path);
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

WebsocketClientSystem::WebsocketClientSystem(const std::string& ca_file) noexcept
	: ca_file_(ca_file)
{
}

void WebsocketClientSystem::SendError(const std::string& msg)
{
	// 直接派发错误事件；某些错误也可能在系统未初始化前发生，
	// 因此此处不检查 initialized_，由回调处进行保护（如果需要）。
	this->SendEvent<WebsocketErrorEvent>(msg);
}

bool WebsocketClientSystem::Connect(const std::string& host, unsigned short port, const std::string& target, bool use_ssl)
{
	const std::string port_str = std::to_string(port);

	try {
		// 调用底层 client.connect（connect 会在内部启动 io 线程并在握手成功后触发回调）
		if (!client.Connect(host, port_str, target, use_ssl, ca_file_)) {
			SendError("[WebsocketClientSystem] Connect failed: Unable to connect to " + host + ":" + std::to_string(port));
			return false;
		}

		return true;
	}
	catch (const std::exception& ex) {
		SendError(std::string("[WebsocketClientSystem] Connect exception: ") + ex.what());
		return false;
	}
}

bool WebsocketClientSystem::Connect(const std::string& path)
{
	try {
		auto parsed = ParseWebsocketUrl(path);
		if (!parsed) {
			SendError(std::string("[WebsocketClientSystem] Connect(path) parse failed: invalid format: ") + path);
			return false;
		}

		return Connect(parsed->host, parsed->port, parsed->target, parsed->use_ssl);
	}
	catch (const std::exception& ex) {
		SendError(std::string("[WebsocketClientSystem] Connect(path) exception: ") + ex.what());
		return false;
	}
}

bool WebsocketClientSystem::Send(const std::string& msg)
{
	if (!client.IsConnected()) {
		SendError("[WebsocketClientSystem] Send failed: not connected");
		return false;
	}
	try {
		if (!client.SendText(msg)) {
			SendError("[WebsocketClientSystem] send_text failed");
			return false;
		}
		return true;
	}
	catch (const std::exception& ex) {
		SendError(std::string("[WebsocketClientSystem] Send exception: ") + ex.what());
		return false;
	}
}

void WebsocketClientSystem::Close()
{
	try {
		client.Disconnect();
	}
	catch (const std::exception& ex) {
		SendError(std::string("[WebsocketClientSystem] Close exception: ") + ex.what());
	}
}

void WebsocketClientSystem::OnInit()
{
	try {
		initialized_.store(true, std::memory_order_release);

		// 将底层回调映射为框架事件
		// 在回调中先检查 initialized_，避免在已反初始化或反初始化期间派发事件
		client.SetConnectionCallback([this]() {
			if (!initialized_.load(std::memory_order_acquire)) return;
			this->SendEvent<WebsocketConnectionEvent>();
		});

		client.SetDisconnectionCallback([this]() {
			if (!initialized_.load(std::memory_order_acquire)) return;
			this->SendEvent<WebsocketDisconnectionEvent>();
		});

		client.SetErrorCallback([this](const std::string& err) {
			// 错误可能发生在连接前/过程中，仍派发以便上层感知
			if (!initialized_.load(std::memory_order_acquire)) {
				// 若尚未初始化，仍派发错误以便诊断
				this->SendEvent<WebsocketErrorEvent>(err);
				return;
			}
			this->SendEvent<WebsocketErrorEvent>(err);
		});

		client.SetMessageCallback([this](const std::string& msg, bool is_binary) {
			if (!initialized_.load(std::memory_order_acquire)) return;
			this->SendEvent<WebsocketReceiveEvent>(msg, is_binary);
		});
	}
	catch (const std::exception& ex) {
		SendError(std::string("[WebsocketClientSystem] OnInit exception: ") + ex.what());
	}
}

void WebsocketClientSystem::OnDeinit()
{
	// 标记为未初始化，回调会检查此标记并停止派发事件
	initialized_.store(false, std::memory_order_release);

	Close();
}

void WebsocketClientSystem::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{
	// 如果需要把事件系统与 websocket 交互，请在此实现
}
