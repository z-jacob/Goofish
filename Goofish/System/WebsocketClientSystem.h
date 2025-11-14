#pragma once
#include "../Helper/JFramework.h"
#include "../Helper/websocket_client.h"

#include <memory>
#include <string>

class WebsocketClientSystem : public JFramework::AbstractSystem
{
public:
    // 外部接口
    bool Connect(const std::string& host, unsigned short port, const std::string& target = "/", bool use_ssl = true);
    bool Send(const std::string& msg);
    void Close();

protected:
    void OnInit() override;
    void OnDeinit() override;
    void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;

private:
    websocket_chat::WebSocketClient client;
};

