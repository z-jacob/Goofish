#pragma once

#ifndef _WIN32_WINNT
// WinHTTP WebSocket requires Windows 8+ for WinHttpWebSocket* APIs
#define _WIN32_WINNT 0x0602
#endif

#include <windows.h>
#include <winhttp.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <memory>
#include <vector>
#include <iostream>

#pragma comment(lib, "winhttp.lib")

class AsyncWSSClient : public std::enable_shared_from_this<AsyncWSSClient>
{
public:
    using MessageHandler = std::function<void(const std::string&)>;
    // CloseHandler receives a Win32 error code (0 for graceful close)
    using CloseHandler = std::function<void(DWORD)>;

    AsyncWSSClient()
        : hSession_(nullptr)
        , hConnect_(nullptr)
        , hWebSocket_(nullptr)
        , running_(false)
    {
    }

    ~AsyncWSSClient()
    {
        Close();
    }

    // connect is asynchronous w.r.t. receiving messages (connect blocks until handshake completes)
    // host/port/target are utf-8 strings (converted internally)
    bool Connect(const std::string& host, unsigned short port, const std::string& target = "/", bool use_ssl = true)
    {
        std::lock_guard<std::mutex> lk(mutex_);

        CloseInternal(); // ensure previous closed

        std::wstring wHost = Utf8ToWide(host);
        std::wstring wTarget = Utf8ToWide(target);

        DWORD flags = WINHTTP_FLAG_BYPASS_PROXY_CACHE;
        if (use_ssl)
            flags |= WINHTTP_FLAG_SECURE;

        hSession_ = WinHttpOpen(L"GoofishAsyncWSS/1.0",
            WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession_)
        {
            CallCloseHandler(GetLastError());
            return false;
        }

        hConnect_ = WinHttpConnect(hSession_, wHost.c_str(), port, 0);
        if (!hConnect_)
        {
            CallCloseHandler(GetLastError());
            CloseInternal();
            return false;
        }

        // Open request
        HINTERNET hRequest = WinHttpOpenRequest(hConnect_, L"GET", wTarget.c_str(),
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hRequest)
        {
            CallCloseHandler(GetLastError());
            CloseInternal();
            return false;
        }

        // Send the request
        BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        if (!ok)
        {
            DWORD err = GetLastError();
            WinHttpCloseHandle(hRequest);
            CallCloseHandler(err);
            CloseInternal();
            return false;
        }

        ok = WinHttpReceiveResponse(hRequest, nullptr);
        if (!ok)
        {
            DWORD err = GetLastError();
            WinHttpCloseHandle(hRequest);
            CallCloseHandler(err);
            CloseInternal();
            return false;
        }

        // Runtime-resolve WinHttpWebSocketCompleteUpgrade to avoid SDK signature issues
        typedef HINTERNET(WINAPI* PFN_WinHttpWebSocketCompleteUpgrade)(HINTERNET);
        HMODULE hWinHttp = GetModuleHandleW(L"winhttp.dll");
        PFN_WinHttpWebSocketCompleteUpgrade pComplete = nullptr;
        if (hWinHttp)
            pComplete = reinterpret_cast<PFN_WinHttpWebSocketCompleteUpgrade>(GetProcAddress(hWinHttp, "WinHttpWebSocketCompleteUpgrade"));

        if (pComplete)
        {
            hWebSocket_ = pComplete(hRequest);
        }
        else
        {
            // If not available at runtime, close request and report
            WinHttpCloseHandle(hRequest);
            CallCloseHandler(ERROR_CALL_NOT_IMPLEMENTED);
            CloseInternal();
            return false;
        }

        // start receiver thread
        running_.store(true);
        recvThread_ = std::thread([this]() { ReceiverLoop(); });

        return true;
    }

    void Send(const std::string& msg)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        if (!hWebSocket_)
        {
            std::cerr << "AsyncWSSClient::Send: no websocket\n";
            return;
        }

        // send as UTF-8 text message
        DWORD res = WinHttpWebSocketSend(hWebSocket_,
            WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
            (LPVOID)msg.data(),
            static_cast<ULONG>(msg.size()));
        if (res != ERROR_SUCCESS)
        {
            CallCloseHandler(res);
        }
    }

    void Close()
    {
        std::lock_guard<std::mutex> lk(mutex_);
        CloseInternal();
    }

    void set_message_handler(MessageHandler h)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        messageHandler_ = std::move(h);
    }

    void set_close_handler(CloseHandler h)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        closeHandler_ = std::move(h);
    }

private:
    static std::wstring Utf8ToWide(const std::string& s)
    {
        if (s.empty()) return std::wstring();
        int size_needed = ::MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
        if (size_needed <= 0) return std::wstring();
        std::wstring w(size_needed, 0);
        ::MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], size_needed);
        return w;
    }

    void ReceiverLoop()
    {
        const ULONG bufSize = 8192;
        std::vector<char> buffer(bufSize);
        while (running_.load())
        {
            ULONG bytesRead = 0;
            WINHTTP_WEB_SOCKET_BUFFER_TYPE bufType = WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE;
            DWORD result = WinHttpWebSocketReceive(hWebSocket_, buffer.data(), bufSize, &bytesRead, &bufType);
            if (result != ERROR_SUCCESS)
            {
                // error or connection closed
                CallCloseHandler(result);
                break;
            }

            if (bufType == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
            {
                // graceful close
                CallCloseHandler(0);
                break;
            }
            else if (bytesRead > 0)
            {
                std::string msg(buffer.data(), buffer.data() + bytesRead);
                MessageHandler h;
                {
                    std::lock_guard<std::mutex> lk(mutex_);
                    h = messageHandler_;
                }
                if (h)
                    h(msg);
            }
        }

        {
            std::lock_guard<std::mutex> lk(mutex_);
            CloseInternal();
        }
    }

    void CloseInternal()
    {
        if (!running_.load() && !hWebSocket_ && !hConnect_ && !hSession_)
            return;

        running_.store(false);

        if (hWebSocket_)
        {
            // request a normal closure (ignore errors)
            WinHttpWebSocketClose(hWebSocket_, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
            WinHttpCloseHandle(hWebSocket_);
            hWebSocket_ = nullptr;
        }

        if (recvThread_.joinable())
            recvThread_.join();

        if (hConnect_)
        {
            WinHttpCloseHandle(hConnect_);
            hConnect_ = nullptr;
        }

        if (hSession_)
        {
            WinHttpCloseHandle(hSession_);
            hSession_ = nullptr;
        }
    }

    void CallCloseHandler(DWORD code)
    {
        CloseHandler h;
        {
            std::lock_guard<std::mutex> lk(mutex_);
            h = closeHandler_;
        }
        if (h)
            h(code);
    }

private:
    std::mutex mutex_;
    HINTERNET hSession_;
    HINTERNET hConnect_;
    HINTERNET hWebSocket_;
    std::thread recvThread_;
    std::atomic<bool> running_;
    MessageHandler messageHandler_;
    CloseHandler closeHandler_;
};