#include "WebSocketClient.h"
#include "Logger.h"
#include <sstream>

LPCSTR g_c_lpszPemCert =
"-----BEGIN CERTIFICATE-----\n"
"MIIDszCCApugAwIBAgIBATANBgkqhkiG9w0BAQsFADB7MQswCQYDVQQGEwJDTjEL\n"
"MAkGA1UECAwCR0QxCzAJBgNVBAcMAkdaMQwwCgYDVQQKDANTU1QxDzANBgNVBAsM\n"
"Bkplc3NtYTETMBEGA1UEAwwKamVzc21hLm9yZzEeMBwGCSqGSIb3DQEJARYPbGRj\n"
"c2FhQDIxY24uY29tMCAXDTI0MDYyNjA1MjUwOFoYDzIyNDMwNzA5MDUyNTA4WjBu\n"
"MQswCQYDVQQGEwJDTjELMAkGA1UECAwCR0QxDDAKBgNVBAoMA1NTVDEPMA0GA1UE\n"
"CwwGSmVzc21hMRMwEQYDVQQDDApqZXNzbWEub3JnMR4wHAYJKoZIhvcNAQkBFg9s\n"
"ZGNzYWFAMjFjbi5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCD\n"
"+MyrJEKCheRoOpMRjR78S8hr9W7XN0/EZWyVKwXRT7EE0aGiQdH/W2a+qpWRMa6E\n"
"Qi47zdBnt0P8ZoFiItQhuhwUJ064afpVoaHHX25UdbF8r+sRTofadughETBBj2Cf\n"
"qh0ia6EOB0QvpJpywWmGZPoMtypjbUiTb/YGOJh2qsVr67MN/E48vt7qt0VxF9SE\n"
"pucvqhraTBljWCeRVCae2c0yBSpq/n+7NhamK7+g3xxCKWRz4pN3wrIoEsXTboTh\n"
"z940caDgthCc23VJ080DN44jZg6c87huKIuxbebJqw2HCM4DwrW+OSzTLszpFAXZ\n"
"yarllOzWnBut20zmYnl1AgMBAAGjTTBLMAkGA1UdEwQCMAAwHQYDVR0OBBYEFJ5E\n"
"RJmJ4pUzEbcU9Yge6nr0oi51MB8GA1UdIwQYMBaAFN49z48DywmoD4cNTQgC6nn2\n"
"QJoUMA0GCSqGSIb3DQEBCwUAA4IBAQBpoSFfDDDKMAy95tSROpYu5WSWQXe6B7kl\n"
"PGJAF6mWe/4b7jHQqDUVkEmFmbMWUAtpTC3P01TrV77dhIosAnC/B76fb7Pto8W4\n"
"cjGpWAT0sSegZuhnLtguTGlnR0vVSh/yRRDEtjN8loWpu3BLWVHYOKnn62QGfY0B\n"
"sRGrfZsKvwB+1w+HOvGopnWv6UYwrzEKthjPMR65rOsoManOv24ua8baJmq0gqF9\n"
"752kD8n703uWUBx79/QlNIPMZC1iUIi1mEjyrTgSag6+3sWAIKihaoF/Nf9d01nw\n"
"iL16EIT5dJ0QJWDCeIxhuTZckw+gL1pBeQU7pqzKHPnvo+8GBnTG\n"
"-----END CERTIFICATE-----\n";


LPCSTR g_c_lpszPemKey =
"-----BEGIN ENCRYPTED PRIVATE KEY-----\n"
"MIIFLTBXBgkqhkiG9w0BBQ0wSjApBgkqhkiG9w0BBQwwHAQIK2UJW9QXIj4CAggA\n"
"MAwGCCqGSIb3DQIJBQAwHQYJYIZIAWUDBAEqBBCDDZQLhAdT91jd6v/5H0+GBIIE\n"
"0PH6tKl+nPi8sU0ryjxDIrHwrT/ZFah+3TAHGE/YFAOZnzRyCFHQTvUZX4p8eSmw\n"
"WOpt5NBUPJ3mT0Ctt7lGBRy4AXSyBrFSamlTruM3P1e3ijluYjMbweZFfCWPq8c/\n"
"jPjbcUkXe6mD96aPSTt/jIunexS8AKovu8c/bFLyTLDk38lATc+GnXQQJ0KiXCRu\n"
"vpjVSKcv2Br6cWqaNTZ71FvH1RmSD6K6givc0w65pKruHYTMApIRR8YC5Y0vx0gD\n"
"6nS12LV/EJEtxTfZFlrzZTRWZISPIzYGuTfS+3bPePlxpbwzhN6vmvgjKhdk+3lN\n"
"3W3ZfqODNhoOKG+mG5Fdj7vR2PU1UND6UUd3+FrzWkXikmalAAwKzRLnyTR1T2rl\n"
"RhM0Qe/HZianeEQTHpCw27gOz1OMw2EKfIEHM6W2BKGOTY5ls5dqgMfP1ZoQUrOr\n"
"59tJo4GpWYFGCuHhTEa5OS/gsgnzymGrkuEwPsdSQaBdzV7lFGTv2/ryKX+vNm9V\n"
"CmKw0nHzOVP19+WL4vPDtbRnLUk8KV9Mg7PdSbGbNcMmTEBk8ju8OvjIUIWZbRTa\n"
"n5C6fhD1DYZcczmlCILYgXyJISu7EDf3z9cKRAf5VbRAedDMB/xHWmrmlxUJ37Kt\n"
"tVgaCD0U6Q3q+3y6OOwugc8UbSo4yA/DbLlG0/U7afwQaNxTLa4HGBQljpoNStIt\n"
"Vgfy2olqHXaf2doSQtsYEl9MHa6neuGfZQMtonDkejnx4KKU+cMhe+KijEUwieYx\n"
"7aoPB71b82XODquDPAL5zOegj0eYgKn5iXyOx5W44S34zfclxtxxgfsDJ3qJ9qoL\n"
"sSenrQ3xAYHJSZRcqEgO31XhoEnkyt1V7G0Bk4/GUMD6uQudr3nsw/ulJpAlNK15\n"
"ZxTSKWrtwOWdwcTj6B14K6wcqMFVNF1Ydbv/qp0b5q5S/orYHzRIPcFmdOAIsjyO\n"
"6na7+D31BH/4pf+TASBNqRNRw5CBqNcGcfiXk11AywxUnmD5ZvC/C0pTpTD/9qC4\n"
"LucWJ0sNAtPq8suFjKqQ+wMvq3rUh050NRm2cm2nUJLxafTnr0v3+kKYbVW8pSWB\n"
"NMelZMVGF1MDYBujg8Mw/xuMhPeLozCZeKmo7eu7aDMXzQMZLfAEJAzU9Du8H4nq\n"
"GgQVUgEkS5rdbjZGkHP0FuM8m8lueKEPDYwHCJv9Be5Z/uxp9OO/Lmdlha0J7gJu\n"
"pihNkAYVxRst96b5okXKooYi/TZxAdThoPYH28VwinGR1I3/8I3M5DbUPIgHhDeB\n"
"ga3u7jt7ZNDUgavukUD0S7WioRb5ooXrXGZ1xmzKLCmMdCDC5S32fQS0wRGfVoMl\n"
"hWbaT+0uak+fOpqVRxSNyE3Ek788ua5iPHaTSXJSoe5lv7OQKDSZ/+wFeLmDPf4M\n"
"BHL2gBLD6RNkz5cWgy14sQcJKNAnyptU4EGPyURZcB8APtB/ITAS2Az/JSxvSBgq\n"
"g/L1FujnP2QEpWpVKkTNxsF867bUPN34KrlPKYjNqcKA2pD4fkFoKSeeNtOEWa++\n"
"d6q9y+mDD97SnIFAAhDFlukzXtyl4MU6uiqRldFiuEt3KzvV19n8M+NyyYIFhfdg\n"
"6TkYEbMJPQ/Y3EGNmyMqbFdJzrdl/B8pr7JQnikTfUZZ\n"
"-----END ENCRYPTED PRIVATE KEY-----\n";


LPCSTR g_c_lpszCAPemCert =
"-----BEGIN CERTIFICATE-----\n"
"MIID2TCCAsGgAwIBAgIUM8TTtPU+ejzffYXCcs/zZsU7OuIwDQYJKoZIhvcNAQEL\n"
"BQAwezELMAkGA1UEBhMCQ04xCzAJBgNVBAgMAkdEMQswCQYDVQQHDAJHWjEMMAoG\n"
"A1UECgwDU1NUMQ8wDQYDVQQLDAZKZXNzbWExEzARBgNVBAMMCmplc3NtYS5vcmcx\n"
"HjAcBgkqhkiG9w0BCQEWD2xkY3NhYUAyMWNuLmNvbTAgFw0yNDA2MjYwNTA0NDNa\n"
"GA8yMjcwMTEyNDA1MDQ0M1owezELMAkGA1UEBhMCQ04xCzAJBgNVBAgMAkdEMQsw\n"
"CQYDVQQHDAJHWjEMMAoGA1UECgwDU1NUMQ8wDQYDVQQLDAZKZXNzbWExEzARBgNV\n"
"BAMMCmplc3NtYS5vcmcxHjAcBgkqhkiG9w0BCQEWD2xkY3NhYUAyMWNuLmNvbTCC\n"
"ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAML+v79+aLQt0Za0dTIZHI5B\n"
"NDs0g5G8bhdOTlW/kNWflaziZ3GY6d6nJSkQ5e29kyFKxlOD6Gls6bOJ86U71u4R\n"
"bCmoFvRTDH4q2cJ/+PbiioLpNveDG6lnRCs9JNRQoJrkpRo6urnVnAdsIf6UFjLI\n"
"dlByNMPGYJ0V8/oKJG5Vu5gcbZV0jVA5+tswkH/zquexEXoKvp18mcwl+pNc/LwW\n"
"0WnGj0uoJjxHg4GsS78PASjhxMR/2d/1OpgPauldFaNHjVPtaLqJnuejwA6M6Sz8\n"
"iFPybAQAMpHL9W8kf08jtbnFvnm4ibUkQL5h+OJoIEQa9AVZOSoFG2/g5Zcn8X8C\n"
"AwEAAaNTMFEwHQYDVR0OBBYEFN49z48DywmoD4cNTQgC6nn2QJoUMB8GA1UdIwQY\n"
"MBaAFN49z48DywmoD4cNTQgC6nn2QJoUMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZI\n"
"hvcNAQELBQADggEBALJnYrYBSZLyYX14FQ04zxG3AX0CtQzNOOa7LDrr+H8Ly+nK\n"
"qS87gg2njMVZH1zM2demtMwydR/F2Ui8ggaduMvc9h5YgQKEwYl8KarJEY03oZoe\n"
"zbQGBxCXpDOtMs1vujzcl/iZbSzwEDF3g4la5U8q4MlmfGFKz9CJbvoxecqYA206\n"
"nNbW2XZsW/xMiQv6iAw5iP/LOR9HAyxcvXIsL790nfcgnTYLmyP254Dj4outc6R+\n"
"PA+f/c1FvkbUBTR5vJt2tsvHcNU218rY2hyOIhDmZeUWprqBO19sUk3scLbVPr3+\n"
"WEWEl2XaCekKuPtAnMgVQuFsocXGyiuIhkOe5Z4=\n"
"-----END CERTIFICATE-----\n";


LPCTSTR g_c_lpszKeyPasswod = "123456";

WebSocketClient* WebSocketClient::m_instance = nullptr;


inline void AddRequestHeader(std::vector<THeader>& headers, const char* name, const char* value) {
	THeader header;
	header.name = name;
	header.value = value;
	headers.push_back(header);
}

inline std::string HttpVersionToString(EnHttpVersion enVersion, char* strResult)
{
	return "HTTP/" + std::to_string(LOBYTE(enVersion)) + "." + std::to_string(HIBYTE(enVersion));
}

BOOL WebSocketClient::SendUpgrade()
{
	std::vector<THeader> headers;
	AddRequestHeader(headers, "Upgrade", "websocket");
	AddRequestHeader(headers, "Connection", "Upgrade");
	AddRequestHeader(headers, "Sec-WebSocket-Key", "x3JJHMbDL1EzLkh9GBhXDw==");
	AddRequestHeader(headers, "Sec-WebSocket-Extensions", "permessage-deflate; client_max_window_bits");
	AddRequestHeader(headers, "Sec-WebSocket-Version", "13");
	return ::HP_HttpClient_SendRequest(m_HttpClient, "GET", "/", headers.data(), (int)headers.size(), nullptr, 0);
}

std::string WebSocketClient::GetHeaderSummary(HP_HttpClient pSender, LPCSTR lpszSep /*= " "*/, int iSepCount /*= 0*/, BOOL bWithContentLength /*= TRUE*/)
{
	// 使用ostringstream提升字符串拼接性能
	std::ostringstream oss;
	std::string SEP1(iSepCount, *lpszSep);
	std::string SEP2 = SEP1 + lpszSep;
	const char* CRLF = "\n";

	// 状态字段
	oss << SEP1 << "[Status Fields]" << CRLF;
	oss << SEP2 << "Status Code: " << ::HP_HttpClient_GetStatusCode(pSender) << CRLF;

	// 响应头
	DWORD dwHeaderCount = 0;
	::HP_HttpClient_GetAllHeaders(pSender, nullptr, &dwHeaderCount);

	oss << SEP1 << "[Response Headers]" << CRLF;

	if (dwHeaderCount == 0)
	{
		oss << SEP2 << "(no header)" << CRLF;
	}
	else
	{
		std::unique_ptr<THeader[]> headers(new THeader[dwHeaderCount]);
		::HP_HttpClient_GetAllHeaders(pSender, headers.get(), &dwHeaderCount);

		for (DWORD i = 0; i < dwHeaderCount; i++)
			oss << SEP2 << headers[i].name << ": " << headers[i].value << CRLF;
	}

	// Cookies
	DWORD dwCookieCount = 0;
	::HP_HttpClient_GetAllCookies(pSender, nullptr, &dwCookieCount);

	oss << SEP1 << "[Cookies]" << CRLF;

	if (dwCookieCount == 0)
	{
		oss << SEP2 << "(no cookie)" << CRLF;
	}
	else
	{
		std::unique_ptr<TCookie[]> cookies(new TCookie[dwCookieCount]);
		::HP_HttpClient_GetAllCookies(pSender, cookies.get(), &dwCookieCount);

		for (DWORD i = 0; i < dwCookieCount; i++)
			oss << SEP2 << cookies[i].name << ": " << cookies[i].value << CRLF;
	}

	// 基本信息
	char versionStr[32] = { 0 };
	::HttpVersionToString((EnHttpVersion)::HP_HttpClient_GetVersion(pSender), versionStr);
	EnHttpUpgradeType enUpgType = ::HP_HttpClient_GetUpgradeType(pSender);
	const char* lpszUpgrade = enUpgType != HUT_NONE ? "true" : "false";
	const char* lpszKeepAlive = ::HP_HttpClient_IsKeepAlive(pSender) ? "true" : "false";

	oss << SEP1 << "[Basic Info]" << CRLF;
	oss << SEP2 << "Version: " << versionStr << CRLF;
	oss << SEP2 << "Status Code: " << ::HP_HttpClient_GetStatusCode(pSender) << CRLF;
	oss << SEP2 << "IsUpgrade: " << lpszUpgrade << CRLF;

	if (enUpgType != HUT_NONE)
		oss << SEP2 << "UpgradeType: " << enUpgType << CRLF;
	oss << SEP2 << "IsKeepAlive: " << lpszKeepAlive << CRLF;

	if (bWithContentLength)
		oss << SEP2 << "ContentLength: " << ::HP_HttpClient_GetContentLength(pSender) << CRLF;

	auto contentType = ::HP_HttpClient_GetContentType(pSender);
	if (contentType != nullptr)
		oss << SEP2 << "Content-Type: " << contentType << CRLF;

	return oss.str();
}

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
		::HP_SSLClient_SetupSSLContextByMemory(m_HttpClient, SSL_VM_NONE, g_c_lpszPemCert, g_c_lpszPemKey, g_c_lpszKeyPasswod, g_c_lpszCAPemCert);
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

	std::string address(szAddress, iAddressLen);
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "local address:" + address + "#" + std::to_string(usPort));
	if (m_instance && m_instance->m_onConnect)
		m_instance->m_onConnect(GetExtraData_s(), dwConnID);

	return HR_OK;
}

EnHandleResult WebSocketClient::OnHandShake(HP_Client pSender, CONNID dwConnID)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s());
	if (m_instance && m_instance->m_onHandShake) {
		m_instance->SendUpgrade();
		m_instance->m_onHandShake(GetExtraData_s(), dwConnID);
	}
	return HR_OK;
}

EnHandleResult WebSocketClient::OnSend(HP_Client pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "(" + std::to_string(iLength) + " bytes)");
	return HR_OK;
}

EnHandleResult WebSocketClient::OnReceive(HP_Client pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "(" + std::to_string(iLength) + " bytes)");
	return HR_OK;
}

EnHandleResult WebSocketClient::OnClose(HP_Client pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	std::string content = "OP: " + std::to_string(enOperation) + ", CODE: " + std::to_string(iErrorCode);
	LOG_INFO(MODULE_INFO, GetExtraData_s() + content);
	if (m_instance && m_instance->m_onClose)
		m_instance->m_onClose(GetExtraData_s(), dwConnID, enOperation, iErrorCode);
	return HR_OK;
}

// ------------------------------------------------------------------------------------------------------------- //

EnHttpParseResult WebSocketClient::OnMessageBegin(HP_HttpClient pSender, CONNID dwConnID)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s());
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnStatusLine(HP_HttpClient pSender, CONNID dwConnID, USHORT usStatusCode, LPCSTR lpszDesc)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "(" + std::to_string(usStatusCode) + ") : " + lpszDesc);
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnHeader(HP_HttpClient pSender, CONNID dwConnID, LPCSTR lpszName, LPCSTR lpszValue)
{
	std::string content = std::string(lpszName) + ": " + std::string(lpszValue);
	LOG_INFO(MODULE_INFO, GetExtraData_s() + content);
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnHeadersComplete(HP_HttpClient pSender, CONNID dwConnID)
{


	auto strSummary = "* * * * * * * * * Summary * * * * * * * * *\n";
	auto strHeaderSummary = GetHeaderSummary(pSender, "    ", 0, TRUE);

	LOG_INFO(MODULE_INFO, GetExtraData_s() + strSummary + strHeaderSummary);
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnBody(HP_HttpClient pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "(" + std::to_string(iLength) + " bytes)");
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnChunkHeader(HP_HttpClient pSender, CONNID dwConnID, int iLength)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "(" + std::to_string(iLength) + " bytes)");
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnChunkComplete(HP_HttpClient pSender, CONNID dwConnID)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s());
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnMessageComplete(HP_HttpClient pSender, CONNID dwConnID)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s());
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnUpgrade(HP_HttpClient pSender, CONNID dwConnID, EnHttpUpgradeType enUpgradeType)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "enUpgradeType:" + std::to_string(enUpgradeType));
	return HPR_OK;
}

EnHttpParseResult WebSocketClient::OnParseError(HP_HttpClient pSender, CONNID dwConnID, int iErrorCode, LPCSTR lpszErrorDesc)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "iErrorCode:" + std::to_string(iErrorCode) + ",lpszErrorDesc:" + lpszErrorDesc);
	return HPR_OK;
}

// ------------------------------------------------------------------------------------------------------------- //

EnHandleResult WebSocketClient::OnWSMessageHeader(HP_HttpClient pSender, CONNID dwConnID, BOOL bFinal, BYTE iReserved, BYTE iOperationCode, const BYTE lpszMask[4], ULONGLONG ullBodyLen)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "bFinal:" + std::to_string(bFinal) + ",iReserved:"
		+ std::to_string(iReserved) + ",iOperationCode:" + std::to_string(iOperationCode) + ",:ullBodyLen" + std::to_string(ullBodyLen));

	if (m_instance && m_instance->m_onWSMessageHeader)
		m_instance->m_onWSMessageHeader(GetExtraData_s(), dwConnID, bFinal, iReserved, iOperationCode, lpszMask, ullBodyLen);

	return HR_OK;
}

EnHandleResult WebSocketClient::OnWSMessageBody(HP_HttpClient pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	auto message = std::string(reinterpret_cast<const char*>(pData), iLength);
	LOG_INFO(MODULE_INFO, GetExtraData_s() + "Data:" + message + ",iLength:" + std::to_string(iLength));
	if (m_instance && m_instance->m_onWSMessageBody)
		m_instance->m_onWSMessageBody(GetExtraData_s(), dwConnID, pData, iLength);


	return HR_OK;
}

EnHandleResult WebSocketClient::OnWSMessageComplete(HP_HttpClient pSender, CONNID dwConnID)
{
	LOG_INFO(MODULE_INFO, GetExtraData_s());
	if (m_instance && m_instance->m_onWSMessageComplete)
		m_instance->m_onWSMessageComplete(GetExtraData_s(), dwConnID);

	BYTE iOperationCode;

	::HP_HttpClient_GetWSMessageState(pSender, nullptr, nullptr, &iOperationCode, nullptr, nullptr, nullptr);

	if (iOperationCode == 0x8)
		return HR_ERROR;

	return HR_OK;
}

std::string WebSocketClient::GetExtraData_s()
{
	std::string extraData = "";
	if (m_instance) {
		extraData = m_instance->GetExtraData();
		extraData = extraData.length() > 0 ? ("[" + extraData + "]") : "";
	}
	return extraData;
}
