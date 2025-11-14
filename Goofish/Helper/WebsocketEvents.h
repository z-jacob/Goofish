#pragma once

#include "JFramework.h"

class WebsocketConnectionEvent : public JFramework::IEvent
{
public:
	bool m_connected;
	WebsocketConnectionEvent(bool connected) {
		m_connected = connected;
	}
};

class WebsocketErrorEvent : public JFramework::IEvent
{
public:
	std::string m_errorMessage;
	WebsocketErrorEvent(std::string& error) {
		m_errorMessage = error;
	}
};

class WebsocketReceiveEvent : public JFramework::IEvent
{
public:
	std::string m_message;
	bool m_isBinary;
	WebsocketReceiveEvent(std::string& message, bool isBinary) {
		m_message = message;
		m_isBinary = isBinary;
	}
};



