#include "HttpClient.h"
#include <string>
#include <sstream>

#pragma comment(lib, "winhttp.lib")

using namespace std;

// Helper: split URL into components using WinHttpCrackUrl
static bool CrackUrl(const wstring& url, URL_COMPONENTS& outComp, std::wstring& host, std::wstring& path, INTERNET_PORT& port, BOOL& isHttps)
{
    ZeroMemory(&outComp, sizeof(outComp));
    outComp.dwStructSize = sizeof(outComp);

    // Request that WinHttpCrackUrl allocate buffers for components
    outComp.dwSchemeLength = (DWORD)-1;
    outComp.dwHostNameLength = (DWORD)-1;
    outComp.dwUrlPathLength = (DWORD)-1;
    outComp.dwExtraInfoLength = (DWORD)-1;

    if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &outComp))
        return false;

    host.assign(outComp.lpszHostName, outComp.dwHostNameLength);
    path.assign(outComp.lpszUrlPath, outComp.dwUrlPathLength);
    if (outComp.dwExtraInfoLength)
        path.append(outComp.lpszExtraInfo, outComp.dwExtraInfoLength);

    port = outComp.nPort;
    isHttps = (outComp.nScheme == INTERNET_SCHEME_HTTPS);
    return true;
}

static void AddRequestHeaders(HINTERNET hRequest, const vector<pair<wstring, wstring>>& headers)
{
    if (headers.empty())
        return;

    // Build a single header block separated by CRLF
    wstring all;
    for (const auto& h : headers) {
        all += h.first + L": " + h.second + L"\r\n";
    }

    WinHttpAddRequestHeaders(hRequest, all.c_str(), (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
}

bool HttpClient::Get(const wstring& url, Response& out, const vector<pair<wstring, wstring>>& headers, DWORD timeoutMs)
{
    out = Response();

    URL_COMPONENTS comp;
    wstring host, path;
    INTERNET_PORT port = 0;
    BOOL isHttps = FALSE;
    if (!CrackUrl(url, comp, host, path, port, isHttps))
        return false;

    HINTERNET hSession = WinHttpOpen(L"GoofishHttpClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // set timeouts
    WinHttpSetTimeouts(hSession, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

    AddRequestHeaders(hRequest, headers);

    BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!ok) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    ok = WinHttpReceiveResponse(hRequest, NULL);
    if (!ok) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD status = 0, size = sizeof(status);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
    out.status = status;

    // Read body
    DWORD dwSize = 0;
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
            break;

        if (dwSize == 0)
            break;

        std::string buffer;
        buffer.resize(dwSize);
        DWORD dwDownloaded = 0;
        if (WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded) && dwDownloaded) {
            out.body.append(buffer.data(), dwDownloaded);
        }
        if (dwDownloaded == 0) break;
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return true;
}

bool HttpClient::Post(const wstring& url, const string& body, Response& out, const vector<pair<wstring, wstring>>& headers, DWORD timeoutMs)
{
    out = Response();

    URL_COMPONENTS comp;
    wstring host, path;
    INTERNET_PORT port = 0;
    BOOL isHttps = FALSE;
    if (!CrackUrl(url, comp, host, path, port, isHttps))
        return false;

    HINTERNET hSession = WinHttpOpen(L"GoofishHttpClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    WinHttpSetTimeouts(hSession, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

    // Add Content-Length header if not provided
    bool hasContentLength = false;
    for (const auto& h : headers) {
        if (_wcsicmp(h.first.c_str(), L"Content-Length") == 0) { hasContentLength = true; break; }
    }

    vector<pair<wstring,wstring>> finalHeaders = headers;
    if (!hasContentLength) {
        finalHeaders.emplace_back(L"Content-Length", to_wstring(body.size()));
    }
    // If no content-type provided, set default
    bool hasContentType = false;
    for (const auto& h : headers) {
        if (_wcsicmp(h.first.c_str(), L"Content-Type") == 0) { hasContentType = true; break; }
    }
    if (!hasContentType) {
        finalHeaders.emplace_back(L"Content-Type", L"application/x-www-form-urlencoded; charset=utf-8");
    }

    AddRequestHeaders(hRequest, finalHeaders);

    BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)body.data(), (DWORD)body.size(), (DWORD)body.size(), 0);
    if (!ok) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    ok = WinHttpReceiveResponse(hRequest, NULL);
    if (!ok) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD status = 0, size = sizeof(status);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
    out.status = status;

    // Read body
    DWORD dwSize = 0;
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
            break;

        if (dwSize == 0)
            break;

        std::string buffer;
        buffer.resize(dwSize);
        DWORD dwDownloaded = 0;
        if (WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded) && dwDownloaded) {
            out.body.append(buffer.data(), dwDownloaded);
        }
        if (dwDownloaded == 0) break;
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return true;
}
