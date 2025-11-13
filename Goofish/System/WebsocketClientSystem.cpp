#include "WebsocketClientSystem.h"

#include "../Helper/SimpleWSSClient.h"
#include <iostream>

bool WebsocketClientSystem::Connect(const std::string& host, unsigned short port, const std::string& target, bool use_ssl)
{
    try
    {
        // 如果已有连接，先关闭
        if (client_)
        {
            client_->Close();
            client_.reset();
        }

        client_ = std::make_shared<AsyncWSSClient>();

        // 应用预设回调（若已设置）
        if (pending_message_handler_)
            client_->set_message_handler(pending_message_handler_);
        if (pending_close_handler_)
            client_->set_close_handler(pending_close_handler_);

        // AsyncWSSClient::Connect 返回 bool 表示是否成功完成握手并启动接收线程
        bool ok = client_->Connect(host, port, target, use_ssl);
        if (!ok)
        {
            // 连接失败，清理 client_
            client_.reset();
        }
        return ok;
    }
    catch (const std::exception& e)
    {
        std::cerr << "WebsocketClientSystem::Connect exception: " << e.what() << std::endl;
        return false;
    }
}

void WebsocketClientSystem::Send(const std::string& msg)
{
    if (client_)
    {
        client_->Send(msg);
    }
    else
    {
        std::cerr << "WebsocketClientSystem::Send: client not connected\n";
    }
}

void WebsocketClientSystem::Close()
{
    if (client_)
    {
        client_->Close();
        client_.reset();
    }
}

void WebsocketClientSystem::SetMessageHandler(AsyncWSSClient::MessageHandler handler)
{
    pending_message_handler_ = std::move(handler);
    if (client_)
        client_->set_message_handler(pending_message_handler_);
}

void WebsocketClientSystem::SetCloseHandler(AsyncWSSClient::CloseHandler handler)
{
    pending_close_handler_ = std::move(handler);
    if (client_)
        client_->set_close_handler(pending_close_handler_);
}

void WebsocketClientSystem::OnInit()
{
    // 按需在系统初始化时自动连接；当前不自动连接，外部调用 Connect(...)
}

void WebsocketClientSystem::OnDeinit()
{
    // 优雅关闭
    Close();
}

void WebsocketClientSystem::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{
    // 如果需要把事件系统与 websocket 交互，请在此实现
}
