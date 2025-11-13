#pragma once
#include "../Helper/JFramework.h"
#include "../Helper/SimpleWSSClient.h"
#include <memory>
#include <string>

class WebsocketClientSystem : public JFramework::AbstractSystem
{
public:
    // 外部接口
    bool Connect(const std::string& host, unsigned short port, const std::string& target = "/", bool use_ssl = true);
    void Send(const std::string& msg);
    void Close();

    void SetMessageHandler(AsyncWSSClient::MessageHandler handler);
    void SetCloseHandler(AsyncWSSClient::CloseHandler handler);

protected:
    void OnInit() override;
    void OnDeinit() override;
    void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;

private:
    std::shared_ptr<AsyncWSSClient> client_;
    AsyncWSSClient::MessageHandler pending_message_handler_;
    AsyncWSSClient::CloseHandler pending_close_handler_;
};

