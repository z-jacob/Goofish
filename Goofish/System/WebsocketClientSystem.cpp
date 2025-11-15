#include "WebsocketClientSystem.h"
#include <iostream>
#include <regex>
#include <cstdlib>
#include <algorithm>
#include <cctype>
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
 * 注意事项：
 * - 底层 client 的回调在 IO 线程执行；本类仅在 OnInit 中将这些回调转发为 JFramework 事件。
 * - 本文件仅添加注释，未更改已有逻辑行为。
 */

bool WebsocketClientSystem::Connect(const std::string& host, unsigned short port, const std::string& target, bool use_ssl)
{
	const std::string port_str = std::to_string(port);
	const std::string ca_file = "cacert.pem";

	try {
		// 调用底层 client.connect（同步调度握手流程），若失败则发送错误事件
		if (!client.connect(host, port_str, target, use_ssl, ca_file)) {
			this->SendEvent<WebsocketErrorEvent>("[WebsocketClientSystem] Connect failed: Unable to connect to " + host + ":" + std::to_string(port));
			return false;
		}
		// 请求底层启动读取循环（connect 之后通常会自动启动，但这里显式调用以确保）
		client.start_read_loop();
		return true;
	}
	catch (const std::exception& ex) {
		std::string err = std::string("[WebsocketClientSystem] Connect exception: ") + ex.what();
		this->SendEvent<WebsocketErrorEvent>(err);
		return false;
	}
}

/**
 * @brief 通过单一 path/url 解析并连接
 *
 * 支持的输入示例：
 * - "ws://example.com:1234/some/path"
 * - "wss://example.com/some/path"
 * - "example.com:1234/path"
 * - "example.com"
 *
 * 解析规则（使用正则）：
 * - 可选 scheme（ws 或 wss）
 * - 主机名（不包含端口和路径）
 * - 可选端口（若缺失则使用默认 80 或 443，取决于 scheme 是否为 wss）
 * - 可选 target（若缺失则默认为 "/"）
 *
 * 成功解析后调用上层的 Connect(host, port, target, use_ssl)。
 */
bool WebsocketClientSystem::Connect(const std::string& path)
{
	// 支持格式示例：
	// ws://example.com:1234/some/path
	// wss://example.com/some/path
	// example.com:1234/path
	// example.com

	try {
		std::string s = path;
		// trim 前后空白
		auto l = s.find_first_not_of(" \t\n\r");
		auto r = s.find_last_not_of(" \t\n\r");
		if (l == std::string::npos) return false;
		s = s.substr(l, r - l + 1);

		// 正则提取：可选 scheme、主机、可选端口、可选路径
		std::regex re(R"(^(?:(ws|wss)://)?([^/:]+)(?::(\d+))?(/.*)?$)", std::regex::icase);
		std::smatch m;
		if (!std::regex_match(s, m, re)) {
			this->SendEvent<WebsocketErrorEvent>(std::string("[WebsocketClientSystem] Connect(path) parse failed: invalid format: ") + path);
			return false;
		}

		// 从 match 中取值（若未匹配则使用默认）
		std::string scheme = m[1].matched ? m[1].str() : "";
		std::string host = m[2].matched ? m[2].str() : "";
		std::string port_str = m[3].matched ? m[3].str() : "";
		std::string target = m[4].matched ? m[4].str() : "/";

		bool use_ssl = false;
		unsigned short port = 0;
		if (!scheme.empty()) {
			// portable, case-insensitive compare
			std::string scheme_l;
			scheme_l.reserve(scheme.size());
			std::transform(scheme.begin(), scheme.end(), std::back_inserter(scheme_l), [](unsigned char ch) { return std::tolower(ch); });
			use_ssl = (scheme_l == "wss");
		}

		if (port_str.empty()) {
			// 如果未指定端口，按 scheme 使用默认端口
			port = use_ssl ? 443 : 80;
		}
		else {
			// 将端口字符串转换为整数并验证范围
			long p = std::strtol(port_str.c_str(), nullptr, 10);
			if (p <= 0 || p > 65535) {
				this->SendEvent<WebsocketErrorEvent>(std::string("[WebsocketClientSystem] Connect(path) invalid port: ") + port_str);
				return false;
			}
			port = static_cast<unsigned short>(p);
		}

		if (target.empty()) target = "/";

		// 委托到按 host/port/target 的 Connect 实现
		return Connect(host, port, target, use_ssl);
	}
	catch (const std::exception& ex) {
		this->SendEvent<WebsocketErrorEvent>(std::string("[WebsocketClientSystem] Connect(path) exception: ") + ex.what());
		return false;
	}
}

/**
 * @brief 发送文本消息
 *
 * 封装底层 client.send_text。若未连接或发送失败则通过事件系统报告错误。
 * 该方法为异步发送：返回 true 表示消息已入队等待发送。
 */
bool WebsocketClientSystem::Send(const std::string& msg)
{
	if (!client.is_connected()) {
		this->SendEvent<WebsocketErrorEvent>("[WebsocketClientSystem] Send failed: not connected");
		return false;
	}
	try {
		if (!client.send_text(msg)) {
			this->SendEvent<WebsocketErrorEvent>("[WebsocketClientSystem] send_text failed");
			return false;
		}
		return true;
	}
	catch (const std::exception& ex) {
		this->SendEvent<WebsocketErrorEvent>(std::string("[WebsocketClientSystem] Send exception: ") + ex.what());
		return false;
	}
}

/**
 * @brief 关闭连接
 *
 * 调用底层 client.disconnect() 并捕获异常，异常信息通过事件上报。
 * disconnect 为异步安全关闭流程（会等待 IO 线程退出）。
 */
void WebsocketClientSystem::Close()
{
	try {
		client.disconnect();
	}
	catch (const std::exception& ex) {
		this->SendEvent<WebsocketErrorEvent>(std::string("[WebsocketClientSystem] Close exception: ") + ex.what());
	}
}

/**
 * @brief 系统初始化回调
 *
 * 在系统启动时被框架调用。此处将底层 client 的回调映射为框架事件：
 * - 连接成功 -> WebsocketConnectionEvent
 * - 断开 -> WebsocketDisconnectionEvent
 * - 错误 -> WebsocketErrorEvent(err)
 * - 收到消息 -> WebsocketReceiveEvent(message, isBinary)
 *
 * 注意：这些回调通常在客户端 IO 线程中被触发，事件分发者应了解线程语义。
 */
void WebsocketClientSystem::OnInit()
{
	try {
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
		this->SendEvent<WebsocketErrorEvent>(std::string("[WebsocketClientSystem] OnInit exception: ") + ex.what());
	}
}

/**
 * @brief 系统反初始化回调
 *
 * 在系统停止/销毁时被调用，确保底层客户端连接被关闭。
 */
void WebsocketClientSystem::OnDeinit()
{
	Close();
}

/**
 * @brief 系统事件回调入口
 *
 * 如果需要将框架事件路由到 WebSocket（例如在收到某类事件时发送消息），
 * 可在此处实现映射逻辑。当前保留为空实现。
 */
void WebsocketClientSystem::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{
	// 如果需要把事件系统与 websocket 交互，请在此实现
}
