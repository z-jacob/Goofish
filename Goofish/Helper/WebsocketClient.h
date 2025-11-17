#pragma once
#include "../HPSocket/HPSocket4C-SSL.h"
#include "../HPSocket/HPTypeDef.h"
#include <string>
#include <windows.h>
#include <functional>



class WebSocketClient
{
public:
    WebSocketClient();
    ~WebSocketClient();

    // 连接服务器（ws 或 wss）
    bool Connect(const std::string& address, USHORT port, bool useSSL = false);

    // 断开连接
    void Disconnect();

    // 发送文本消息
    bool SendText(const std::string& text);

    // 发送二进制消息
    bool SendBinary(const BYTE* data, int length);

    // 发送关闭帧
    void SendClose();

    // 是否已连接
    bool IsConnected() const { return m_connected; }

    void SetExtraData(const std::string& extraData) { m_extraData = extraData; }
    std::string GetExtraData() const { return m_extraData; }

	// 回调设置
	void SetOnConnect(const std::function<void(std::string,CONNID)>& cb) { m_onConnect = cb; }
	void SetOnHandShake(const std::function<void(std::string, CONNID)>& cb) { m_onHandShake = cb; }
	void SetOnWSMessageHeader(const std::function<void(std::string, CONNID, BOOL, BYTE, BYTE, const BYTE*, ULONGLONG)>& cb) { m_onWSMessageHeader = cb; }
	void SetOnWSMessageBody(const std::function<void(std::string, CONNID, const BYTE*, int)>& cb) { m_onWSMessageBody = cb; }
	void SetOnWSMessageComplete(const std::function<void(std::string, CONNID)>& cb) { m_onWSMessageComplete = cb; }
	void SetOnClose(const std::function<void(std::string, CONNID, EnSocketOperation, int)>& cb) { m_onClose = cb; }


private:
    static EnHandleResult __stdcall OnConnect(HP_Client sender, CONNID connID);
    static EnHandleResult __stdcall OnHandShake(HP_Client sender, CONNID connID);
    static EnHandleResult __stdcall OnWSMessageHeader(HP_HttpClient sender, CONNID connID, BOOL bFinal, BYTE iReserved, BYTE iOperationCode, const BYTE lpszMask[4], ULONGLONG ullBodyLen);
    static EnHandleResult __stdcall OnWSMessageBody(HP_HttpClient sender, CONNID connID, const BYTE* pData, int iLength);
    static EnHandleResult __stdcall OnWSMessageComplete(HP_HttpClient sender, CONNID connID);
    static EnHandleResult __stdcall OnClose(HP_Client sender, CONNID connID, EnSocketOperation enOperation, int iErrorCode);

    HP_HttpClientListener m_listener;
    HP_HttpClient m_client;
    bool m_connected;
    bool m_useSSL;
    std::string m_extraData;

	// 回调成员
	static WebSocketClient* m_instance;
	std::function<void(std::string, CONNID)> m_onConnect;
	std::function<void(std::string, CONNID)> m_onHandShake;
	std::function<void(std::string, CONNID, BOOL, BYTE, BYTE, const BYTE*, ULONGLONG)> m_onWSMessageHeader;
	std::function<void(std::string, CONNID, const BYTE*, int)> m_onWSMessageBody;
	std::function<void(std::string, CONNID)> m_onWSMessageComplete;
	std::function<void(std::string, CONNID, EnSocketOperation, int)> m_onClose;
};