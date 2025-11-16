#include "WebSocketClient.h"
#include "Logger.h"

WebSocketClient* WebSocketClient::m_instance = nullptr;


WebSocketClient::WebSocketClient()
	: m_listener(nullptr), m_client(nullptr), m_connected(false), m_useSSL(false)
{
	m_instance = this;
	m_listener = ::Create_HP_HttpClientListener();
	::HP_Set_FN_HttpClient_OnConnect(m_listener, OnConnect);
	::HP_Set_FN_HttpClient_OnHandShake(m_listener, OnHandShake);
	::HP_Set_FN_HttpClient_OnWSMessageHeader(m_listener, OnWSMessageHeader);
	::HP_Set_FN_HttpClient_OnWSMessageBody(m_listener, OnWSMessageBody);
	::HP_Set_FN_HttpClient_OnWSMessageComplete(m_listener, OnWSMessageComplete);
	::HP_Set_FN_HttpClient_OnClose(m_listener, OnClose);
}

WebSocketClient::~WebSocketClient()
{
	Disconnect();
	if (m_client) ::Destroy_HP_HttpClient(m_client);
	if (m_listener) ::Destroy_HP_HttpClientListener(m_listener);
}

bool WebSocketClient::Connect(const std::string& address, USHORT port, bool useSSL)
{
	m_useSSL = useSSL;
	if (m_client) ::Destroy_HP_HttpClient(m_client);

	if (useSSL)
	{
		m_client = ::Create_HP_HttpsClient(m_listener);
		// SSL 初始化参数请根据实际情况填写
		::HP_SSLClient_SetupSSLContext(m_client, SSL_VM_NONE, nullptr, nullptr, nullptr, nullptr);
	}
	else
	{
		m_client = ::Create_HP_HttpClient(m_listener);
	}

	// 启动连接
#ifdef UNICODE
	std::wstring waddress(address.begin(), address.end());
	m_connected = ::HP_Client_Start(m_client, waddress.c_str(), port, TRUE);
#else
	m_connected = ::HP_Client_Start(m_client, address.c_str(), port, TRUE);
#endif
	return m_connected;
}

void WebSocketClient::Disconnect()
{
	if (m_client && m_connected)
	{
		::HP_Client_Stop(m_client);
		m_connected = false;
	}
}

bool WebSocketClient::SendText(const std::string& text)
{
	static const BYTE MASK_KEY[] = { 0x1, 0x2, 0x3, 0x4 };
	if (!m_connected) return false;
#ifdef UNICODE
	std::wstring wtext(text.begin(), text.end());
	return ::HP_HttpClient_SendWSMessage(m_client, TRUE, 0, 0x1, MASK_KEY, (BYTE*)wtext.c_str(), (int)(wtext.length() * sizeof(wchar_t)), 0);
#else
	return ::HP_HttpClient_SendWSMessage(m_client, TRUE, 0, 0x1, MASK_KEY, (BYTE*)text.c_str(), (int)text.length(), 0);
#endif
}

bool WebSocketClient::SendBinary(const BYTE* data, int length)
{
	static const BYTE MASK_KEY[] = { 0x1, 0x2, 0x3, 0x4 };
	if (!m_connected) return false;
	return ::HP_HttpClient_SendWSMessage(m_client, TRUE, 0, 0x2, MASK_KEY, data, length, 0);
}

void WebSocketClient::SendClose()
{
	static const BYTE MASK_KEY[] = { 0x1, 0x2, 0x3, 0x4 };
	if (m_connected)
		::HP_HttpClient_SendWSMessage(m_client, TRUE, 0, 0x8, MASK_KEY, nullptr, 0, 0);
}

// 回调实现
EnHandleResult __stdcall WebSocketClient::OnConnect(HP_Client sender, CONNID connID)
{
	LOG_INFO(MODULE_INFO + "connID:" + std::to_string(connID));
	if (m_instance && m_instance->m_onConnect) m_instance->m_onConnect(connID);
	return HR_OK;
}

EnHandleResult __stdcall WebSocketClient::OnHandShake(HP_Client sender, CONNID connID)
{
	LOG_INFO(MODULE_INFO + "connID:" + std::to_string(connID));
	if (m_instance && m_instance->m_onHandShake) m_instance->m_onHandShake(connID);
	return HR_OK;
}

EnHandleResult __stdcall WebSocketClient::OnWSMessageHeader(HP_HttpClient sender, CONNID connID, BOOL bFinal, BYTE iReserved, BYTE iOperationCode, const BYTE lpszMask[4], ULONGLONG ullBodyLen)
{
	LOG_INFO(MODULE_INFO + "connID:" + std::to_string(connID) + "," + std::to_string(bFinal) + "," + std::to_string(iReserved) + "," + std::to_string(iOperationCode) + "," + std::to_string(ullBodyLen));
	if (m_instance && m_instance->m_onWSMessageHeader) m_instance->m_onWSMessageHeader(connID, bFinal, iReserved, iOperationCode, lpszMask, ullBodyLen);
	return HR_OK;
}

EnHandleResult __stdcall WebSocketClient::OnWSMessageBody(HP_HttpClient sender, CONNID connID, const BYTE* pData, int iLength)
{
	auto message = std::string(reinterpret_cast<const char*>(pData), iLength);
	LOG_INFO(MODULE_INFO + "connID:" + std::to_string(connID) + "," + std::to_string(iLength) + "#" + message);
	if (m_instance && m_instance->m_onWSMessageBody) m_instance->m_onWSMessageBody(connID, pData, iLength);
	return HR_OK;
}

EnHandleResult __stdcall WebSocketClient::OnWSMessageComplete(HP_HttpClient sender, CONNID connID)
{
	LOG_INFO(MODULE_INFO + "connID:" + std::to_string(connID));
	if (m_instance && m_instance->m_onWSMessageComplete) m_instance->m_onWSMessageComplete(connID);
	return HR_OK;
}

EnHandleResult __stdcall WebSocketClient::OnClose(HP_Client sender, CONNID connID, EnSocketOperation enOperation, int iErrorCode)
{
	LOG_INFO(MODULE_INFO + "connID:" + std::to_string(connID) + "," + std::to_string(enOperation) + "," + std::to_string(iErrorCode));
	if (m_instance && m_instance->m_onClose) m_instance->m_onClose(connID, enOperation, iErrorCode);
	return HR_OK;
}