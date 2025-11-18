#include "WebSocketClient.h"
#include "Logger.h"

WebSocketClient* WebSocketClient::m_instance = nullptr;


WebSocketClient::WebSocketClient()
	: m_HttpClientListener(nullptr), m_HttpClient(nullptr), m_connected(false), m_useSSL(false)
{
	m_instance = this;

	m_HttpClientListener = ::Create_HP_HttpClientListener();


	// 设置 HTTP 监听器回调函数
	::HP_Set_FN_HttpClient_OnConnect(m_HttpClientListener, OnConnect);
	::HP_Set_FN_HttpClient_OnHandShake(m_HttpClientListener, OnHandShake);
	::HP_Set_FN_HttpClient_OnSend(m_HttpClientListener, OnSend);
	::HP_Set_FN_HttpClient_OnReceive(m_HttpClientListener, OnReceive);
	::HP_Set_FN_HttpClient_OnClose(m_HttpClientListener, OnClose);

	::HP_Set_FN_HttpClient_OnMessageBegin(m_HttpClientListener, OnMessageBegin);
	::HP_Set_FN_HttpClient_OnStatusLine(m_HttpClientListener, OnStatusLine);
	::HP_Set_FN_HttpClient_OnHeader(m_HttpClientListener, OnHeader);
	::HP_Set_FN_HttpClient_OnHeadersComplete(m_HttpClientListener, OnHeadersComplete);
	::HP_Set_FN_HttpClient_OnBody(m_HttpClientListener, OnBody);
	::HP_Set_FN_HttpClient_OnChunkHeader(m_HttpClientListener, OnChunkHeader);
	::HP_Set_FN_HttpClient_OnChunkComplete(m_HttpClientListener, OnChunkComplete);
	::HP_Set_FN_HttpClient_OnMessageComplete(m_HttpClientListener, OnMessageComplete);
	::HP_Set_FN_HttpClient_OnUpgrade(m_HttpClientListener, OnUpgrade);
	::HP_Set_FN_HttpClient_OnParseError(m_HttpClientListener, OnParseError);

	::HP_Set_FN_HttpClient_OnWSMessageHeader(m_HttpClientListener, OnWSMessageHeader);
	::HP_Set_FN_HttpClient_OnWSMessageBody(m_HttpClientListener, OnWSMessageBody);
	::HP_Set_FN_HttpClient_OnWSMessageComplete(m_HttpClientListener, OnWSMessageComplete);
}

WebSocketClient::~WebSocketClient()
{
	Disconnect();
	if (m_HttpClient) ::Destroy_HP_HttpClient(m_HttpClient);
	if (m_HttpClientListener) ::Destroy_HP_HttpClientListener(m_HttpClientListener);
}

bool WebSocketClient::Connect(const std::string& address, USHORT port, bool useSSL)
{
	m_useSSL = useSSL;
	if (m_HttpClient) ::Destroy_HP_HttpClient(m_HttpClient);

	if (useSSL)
	{
		m_HttpClient = ::Create_HP_HttpsClient(m_HttpClientListener);
		// SSL 初始化参数请根据实际情况填写
		::HP_SSLClient_SetupSSLContext(m_HttpClient, SSL_VM_NONE, nullptr, nullptr, nullptr, nullptr);
	}
	else
	{
		m_HttpClient = ::Create_HP_HttpClient(m_HttpClientListener);
	}

	// 启动连接
#ifdef UNICODE
	std::wstring waddress(address.begin(), address.end());
	m_connected = ::HP_Client_Start(m_client, waddress.c_str(), port, TRUE);
#else
	m_connected = ::HP_Client_Start(m_HttpClient, address.c_str(), port, TRUE);
#endif
	return m_connected;
}

void WebSocketClient::Disconnect()
{
	if (m_HttpClient && m_connected)
	{
		::HP_Client_Stop(m_HttpClient);
		m_connected = false;
	}
}

bool WebSocketClient::SendText(const std::string& text)
{
	static const BYTE MASK_KEY[] = { 0x1, 0x2, 0x3, 0x4 };
	if (!m_connected) return false;
#ifdef UNICODE
	std::wstring wtext(text.begin(), text.end());
	return ::HP_HttpClient_SendWSMessage(m_HttpClient, TRUE, 0, 0x1, MASK_KEY, (BYTE*)wtext.c_str(), (int)(wtext.length() * sizeof(wchar_t)), 0);
#else
	return ::HP_HttpClient_SendWSMessage(m_HttpClient, TRUE, 0, 0x1, MASK_KEY, (BYTE*)text.c_str(), (int)text.length(), 0);
#endif
}

bool WebSocketClient::SendBinary(const BYTE* data, int length)
{
	static const BYTE MASK_KEY[] = { 0x1, 0x2, 0x3, 0x4 };
	if (!m_connected) return false;
	return ::HP_HttpClient_SendWSMessage(m_HttpClient, TRUE, 0, 0x2, MASK_KEY, data, length, 0);
}

void WebSocketClient::SendClose()
{
	static const BYTE MASK_KEY[] = { 0x1, 0x2, 0x3, 0x4 };
	if (m_connected)
		::HP_HttpClient_SendWSMessage(m_HttpClient, TRUE, 0, 0x8, MASK_KEY, nullptr, 0, 0);
}

// 回调实现
EnHandleResult WebSocketClient::OnConnect(HP_Client pSender, CONNID dwConnID)
{
	char szAddress[100];
	int iAddressLen = sizeof(szAddress) / sizeof(char);
	USHORT usPort;

	::HP_Client_GetLocalAddress(pSender, szAddress, &iAddressLen, &usPort);

	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}

	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",szAddress:" + szAddress + ",usPort:" + std::to_string(usPort));

	if (m_instance && m_instance->m_onConnect) m_instance->m_onConnect(extraData, dwConnID);

	return HR_OK;
}

EnHandleResult WebSocketClient::OnHandShake(HP_Client pSender, CONNID dwConnID)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID));
	if (m_instance && m_instance->m_onHandShake) m_instance->m_onHandShake(extraData, dwConnID);

	return HR_OK;
}

EnHandleResult WebSocketClient::OnSend(HP_Client pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}

	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",iLength:" + std::to_string(iLength));

	return HR_OK;
}

EnHandleResult WebSocketClient::OnReceive(HP_Client pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}

	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID));

	return HR_OK;
}

EnHandleResult WebSocketClient::OnClose(HP_Client pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",enOperation:" + std::to_string(enOperation) + ",iErrorCode:" + std::to_string(iErrorCode));
	if (m_instance && m_instance->m_onClose) m_instance->m_onClose(extraData, dwConnID, enOperation, iErrorCode);

	return HR_OK;
}

// ------------------------------------------------------------------------------------------------------------- //

EnHttpParseResult WebSocketClient::OnMessageBegin(HP_HttpClient pSender, CONNID dwConnID)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID));
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnStatusLine(HP_HttpClient pSender, CONNID dwConnID, USHORT usStatusCode, LPCSTR lpszDesc)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",usStatusCode:" + std::to_string(usStatusCode));

	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnHeader(HP_HttpClient pSender, CONNID dwConnID, LPCSTR lpszName, LPCSTR lpszValue)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID));
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnHeadersComplete(HP_HttpClient pSender, CONNID dwConnID)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID));

	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnBody(HP_HttpClient pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",iLength:" + std::to_string(iLength));
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnChunkHeader(HP_HttpClient pSender, CONNID dwConnID, int iLength)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",iLength:" + std::to_string(iLength));
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnChunkComplete(HP_HttpClient pSender, CONNID dwConnID)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID));

	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnMessageComplete(HP_HttpClient pSender, CONNID dwConnID)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID));
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnUpgrade(HP_HttpClient pSender, CONNID dwConnID, EnHttpUpgradeType enUpgradeType)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",enUpgradeType:" + std::to_string(enUpgradeType));
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnParseError(HP_HttpClient pSender, CONNID dwConnID, int iErrorCode, LPCSTR lpszErrorDesc)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",iErrorCode:" + std::to_string(iErrorCode) + ",lpszErrorDesc:" + lpszErrorDesc);

	return HPR_OK;
}

// ------------------------------------------------------------------------------------------------------------- //

EnHandleResult WebSocketClient::OnWSMessageHeader(HP_HttpClient pSender, CONNID dwConnID, BOOL bFinal, BYTE iReserved, BYTE iOperationCode, const BYTE lpszMask[4], ULONGLONG ullBodyLen)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}

	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",bFinal:" + std::to_string(bFinal) + ",iReserved:"
		+ std::to_string(iReserved) + ",iOperationCode:" + std::to_string(iOperationCode) + ",:ullBodyLen" + std::to_string(ullBodyLen));

	if (m_instance && m_instance->m_onWSMessageHeader)
		m_instance->m_onWSMessageHeader(extraData, dwConnID, bFinal, iReserved, iOperationCode, lpszMask, ullBodyLen);

	return HR_OK;
}

EnHandleResult WebSocketClient::OnWSMessageBody(HP_HttpClient pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	auto message = std::string(reinterpret_cast<const char*>(pData), iLength);
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID) + ",Data:" + message + ",iLength:" + std::to_string(iLength));
	if (m_instance && m_instance->m_onWSMessageBody) m_instance->m_onWSMessageBody(extraData, dwConnID, pData, iLength);


	return HR_OK;
}

EnHandleResult WebSocketClient::OnWSMessageComplete(HP_HttpClient pSender, CONNID dwConnID)
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	LOG_INFO(MODULE_INFO + extraData + "connID:" + std::to_string(dwConnID));
	if (m_instance && m_instance->m_onWSMessageComplete) m_instance->m_onWSMessageComplete(extraData, dwConnID);

	BYTE iOperationCode;

	::HP_HttpClient_GetWSMessageState(pSender, nullptr, nullptr, &iOperationCode, nullptr, nullptr, nullptr);

	if (iOperationCode == 0x8)
		return HR_ERROR;

	return HR_OK;
}
