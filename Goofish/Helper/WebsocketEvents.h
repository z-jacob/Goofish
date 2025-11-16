#pragma once
#include <string>
#include "JFramework.h"
#include "../HPSocket/HPTypeDef.h"

class WebsocketConnectionEvent : public JFramework::IEvent
{
public:
	CONNID m_dwConnID;
	WebsocketConnectionEvent(CONNID dwConnID) : m_dwConnID(m_dwConnID) {
	}
};


class WebsocketHandShakeEvent : public JFramework::IEvent
{
public:
	CONNID m_dwConnID;
	WebsocketHandShakeEvent(CONNID dwConnID) : m_dwConnID(m_dwConnID) {
	}
};

class WebsocketMessageCompleteEvent : public JFramework::IEvent
{
public:
	CONNID m_dwConnID;
	WebsocketMessageCompleteEvent(CONNID dwConnID) : m_dwConnID(m_dwConnID) {
	}
};

class WebsocketMessageBodyEvent : public JFramework::IEvent
{
public:
	CONNID m_dwConnID;
	std::string m_message;
	WebsocketMessageBodyEvent(CONNID dwConnID, std::string message) : m_dwConnID(dwConnID), m_message(message) {
	}
};

class WebsocketDisconnectionEvent : public JFramework::IEvent
{
public:
	CONNID m_dwConnID;
	WebsocketDisconnectionEvent(CONNID dwConnID) : m_dwConnID(m_dwConnID) {
	}
};

class WebsocketErrorEvent : public JFramework::IEvent
{
public:
	std::string m_errorMessage;
	WebsocketErrorEvent(std::string error) {
		m_errorMessage = error;
	}
};

class WebsocketReceiveEvent : public JFramework::IEvent
{
public:
	std::string m_message;
	bool m_isBinary;
	WebsocketReceiveEvent(std::string message, bool isBinary) {
		m_message = message;
		m_isBinary = isBinary;
	}
};



