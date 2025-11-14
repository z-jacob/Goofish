#pragma once
#include "../Helper/JFramework.h"
#include "../Helper/websocket_client.h"

#include <memory>
#include <string>

class WebsocketClientSystem : public JFramework::AbstractSystem
{
public:
    WebsocketClientSystem() = default;
    virtual ~WebsocketClientSystem() noexcept override = default;

    // 新接口：传入一个 path/url（例如: "ws://example.com:1234/some/path" 或 "example.com:1234/path"）
    // 函数会解析 host, port, target 与是否使用 ssl
    bool Connect(const std::string& path);

    bool Send(const std::string& msg);
    void Close();

protected:
    void OnInit() override;
    void OnDeinit() override;
    void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;

    // 外部接口
    // 兼容旧接口：按 host/port/target/use_ssl 连接
    bool Connect(const std::string& host, unsigned short port, const std::string& target = "/", bool use_ssl = true);
private:
    websocket_chat::WebSocketClient client;
};

