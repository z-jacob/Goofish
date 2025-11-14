#include "WebsocketClientSystem.h"
#include <iostream>
#include "../Helper/WebsocketEvents.h"

bool WebsocketClientSystem::Connect(const std::string& host, unsigned short port, const std::string& target, bool use_ssl)
{
	// 将端口转为字符串并尝试连接
	const std::string port_str = std::to_string(port);
	// 若需要自定义 CA bundle，请修改此处或扩展接口以传入 ca_file
	const std::string ca_file = "cacert.pem";

	// 这里将 Connect 的 use_ssl 参数映射为 websocket_client.connect 的 verify_peer 参数。
	// websocket_client 会根据 host/port 自动检测是否使用 SSL。
	if (!client.connect(host, port_str, target, use_ssl, ca_file)) {
		std::cerr << "connect failed" << std::endl;
		return false;
	}

	client.start_read_loop();
	return true;
}

bool WebsocketClientSystem::Send(const std::string& msg)
{
	if (!client.is_connected()) {
		std::cerr << "Send failed: not connected" << std::endl;
		return false;
	}

	if (!client.send_text(msg)) {
		std::cerr << "send_text failed" << std::endl;
		return false;
	}
	return true;
}

void WebsocketClientSystem::Close()
{
	client.disconnect();
}


void WebsocketClientSystem::OnInit()
{
	// 设置回调
	client.set_connection_callback([this](bool connected) {
		this->SendEvent<WebsocketConnectionEvent>(connected);
		});

	client.set_error_callback([this](const std::string& err) {
		this->SendEvent<WebsocketErrorEvent>(err);
		});

	client.set_message_callback([this](const std::string& msg, bool is_binary) {
		this->SendEvent<WebsocketReceiveEvent>(msg, is_binary);
		});
}

void WebsocketClientSystem::OnDeinit()
{
	Close();
}

void WebsocketClientSystem::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{
	// 如果需要把事件系统与 websocket 交互，请在此实现
}
