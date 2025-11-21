#pragma once

#include <string>
#include <vector>
#include <utility>
#include <windows.h>
#include <winhttp.h>

// Simple synchronous HTTP client wrapper using WinHTTP.
// - Supports GET and POST
// - Returns status code and response body
// Note: this is a minimal safe wrapper for use in Windows desktop apps.

class HttpClient {
public:
    struct Response {
        DWORD status = 0;
        std::string body;
    };

    // headers: vector of (name, value), name/value are wide strings (UTF-16)
    static bool Get(const std::wstring& url,
                    Response& out,
                    const std::vector<std::pair<std::wstring, std::wstring>>& headers = {},
                    DWORD timeoutMs = 30000);

    // body is raw bytes (typically UTF-8). headers similarly provided.
    static bool Post(const std::wstring& url,
                     const std::string& body,
                     Response& out,
                     const std::vector<std::pair<std::wstring, std::wstring>>& headers = {},
                     DWORD timeoutMs = 30000);
};
