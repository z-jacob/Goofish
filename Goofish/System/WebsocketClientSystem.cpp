#include "WebsocketClientSystem.h"
#include <iostream>
#include <regex>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include "../Helper/WebsocketEvents.h"

bool WebsocketClientSystem::Connect(const std::string& host, unsigned short port, const std::string& target, bool use_ssl)
{
	const std::string port_str = std::to_string(port);
	const std::string ca_file = "cacert.pem";

	try {
		if (!client.connect(host, port_str, target, use_ssl, ca_file)) {
			std::cerr << "[WebsocketClientSystem] Connect failed: Unable to connect to " << host << ":" << port << std::endl;
			return false;
		}
		client.start_read_loop();
		return true;
	} catch (const std::exception& ex) {
		std::cerr << "[WebsocketClientSystem] Connect exception: " << ex.what() << std::endl;
		return false;
	}
}

// 新增：通过传入单个 path/url 解析 host、port、target、use_ssl
bool WebsocketClientSystem::Connect(const std::string& path)
{
	// 支持格式示例：
	// ws://example.com:1234/some/path
	// wss://example.com/some/path
	// example.com:1234/path
	// example.com

	try {
		std::string s = path;
		// trim
		auto l = s.find_first_not_of(" \t\n\r");
		auto r = s.find_last_not_of(" \t\n\r");
		if (l == std::string::npos) return false;
		s = s.substr(l, r - l + 1);

		std::regex re(R"(^(?:(ws|wss)://)?([^/:]+)(?::(\d+))?(/.*)?$)", std::regex::icase);
		std::smatch m;
		if (!std::regex_match(s, m, re)) {
			std::cerr << "[WebsocketClientSystem] Connect(path) parse failed: invalid format: " << path << std::endl;
			return false;
		}

		std::string scheme = m[1].matched ? m[1].str() : "";
		std::string host = m[2].matched ? m[2].str() : "";
		std::string port_str = m[3].matched ? m[3].str() : "";
		std::string target = m[4].matched ? m[4].str() : "/";

		bool use_ssl = false;
		unsigned short port = 0;
		if (!scheme.empty()) {
			// portable case-insensitive compare
			std::string scheme_l;
			scheme_l.reserve(scheme.size());
			std::transform(scheme.begin(), scheme.end(), std::back_inserter(scheme_l), [](unsigned char ch){ return std::tolower(ch); });
			use_ssl = (scheme_l == "wss");
		}

		if (port_str.empty()) {
			// 默认端口
			port = use_ssl ? 443 : 80;
		} else {
			long p = std::strtol(port_str.c_str(), nullptr, 10);
			if (p <= 0 || p > 65535) {
				std::cerr << "[WebsocketClientSystem] Connect(path) invalid port: " << port_str << std::endl;
				return false;
			}
			port = static_cast<unsigned short>(p);
		}

		if (target.empty()) target = "/";

		return Connect(host, port, target, use_ssl);
	} catch (const std::exception& ex) {
		std::cerr << "[WebsocketClientSystem] Connect(path) exception: " << ex.what() << std::endl;
		return false;
	}
}

bool WebsocketClientSystem::Send(const std::string& msg)
{
	if (!client.is_connected()) {
		std::cerr << "[WebsocketClientSystem] Send failed: not connected" << std::endl;
		return false;
	}
	try {
		if (!client.send_text(msg)) {
			std::cerr << "[WebsocketClientSystem] send_text failed" << std::endl;
			return false;
		}
		return true;
	} catch (const std::exception& ex) {
		std::cerr << "[WebsocketClientSystem] Send exception: " << ex.what() << std::endl;
		return false;
	}
}

void WebsocketClientSystem::Close()
{
	try {
		client.disconnect();
	} catch (const std::exception& ex) {
		std::cerr << "[WebsocketClientSystem] Close exception: " << ex.what() << std::endl;
	}
}

void WebsocketClientSystem::OnInit()
{
	try {
		client.set_connection_callback([this](bool connected) {
			this->SendEvent<WebsocketConnectionEvent>(connected);
		});

		client.set_error_callback([this](const std::string& err) {
			this->SendEvent<WebsocketErrorEvent>(err);
		});

		client.set_message_callback([this](const std::string& msg, bool is_binary) {
			this->SendEvent<WebsocketReceiveEvent>(msg, is_binary);
		});
	} catch (const std::exception& ex) {
		std::cerr << "[WebsocketClientSystem] OnInit exception: " << ex.what() << std::endl;
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
