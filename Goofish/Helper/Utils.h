#pragma once
#include <string>
#include <string_view>
#include <chrono>
#include <windows.h>
#include <wincrypt.h>

class Utils {
public:
	static float Utils::GetDpi()
	{
		HDC hDC = ::GetDC(nullptr); // 获取整个屏幕的 DC
		int dpiX = ::GetDeviceCaps(hDC, LOGPIXELSX); // 水平 DPI
		int dpiY = ::GetDeviceCaps(hDC, LOGPIXELSY); // 垂直 DPI
		::ReleaseDC(nullptr, hDC);
		return dpiY / 96.0f;
	}

	// Convert UTF-8 std::string to UTF-16 std::wstring
	static std::wstring StringToWString(const std::string& s)
	{
		if (s.empty())
			return {};
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), NULL, 0);
		if (size_needed <= 0)
			return {};
		std::wstring w;
		w.resize(size_needed);
		MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), &w[0], size_needed);
		return w;
	}

	// Convert UTF-16 std::wstring to UTF-8 std::string
	static std::string WStringToString(const std::wstring& ws)
	{
		if (ws.empty())
			return {};
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.data(), static_cast<int>(ws.size()), NULL, 0, NULL, NULL);
		if (size_needed <= 0)
			return {};
		std::string s;
		s.resize(size_needed);
		WideCharToMultiByte(CP_UTF8, 0, ws.data(), static_cast<int>(ws.size()), &s[0], size_needed, NULL, NULL);
		return s;
	}

	// Extract substring between two delimiters
	static std::string ExtractBetween(std::string_view src, std::string_view left, std::string_view right)
	{
		size_t pos1 = src.find(left);
		if (pos1 == std::string_view::npos) return {};
		pos1 += left.size();
		size_t pos2 = src.find(right, pos1);
		if (pos2 == std::string_view::npos || pos2 <= pos1) return {};
		return std::string(src.substr(pos1, pos2 - pos1));
	}

	// Get current 10-digit timestamp (seconds since epoch, left-padded with zeros)
	static std::string GetTimestamp10()
	{
		using namespace std::chrono;
		auto now = system_clock::now();
		auto ts = duration_cast<seconds>(now.time_since_epoch()).count();
		std::string s = std::to_string(ts);
		if (s.size() < 10) s = std::string(10 - s.size(), '0') + s;
		else if (s.size() > 10) s = s.substr(0, 10);
		return s;
	}

	// Get current 13-digit timestamp (milliseconds since epoch, left-padded with zeros)
	static std::string GetTimestamp13()
	{
		using namespace std::chrono;
		auto now = system_clock::now();
		auto ts = duration_cast<milliseconds>(now.time_since_epoch()).count();
		std::string s = std::to_string(ts);
		if (s.size() < 13) s = std::string(13 - s.size(), '0') + s;
		else if (s.size() > 13) s = s.substr(0, 13);
		return s;
	}

	// Calculate MD5 hash of a string, return as hex string
	static std::string MD5Hash(const std::string& data)
	{
		HCRYPTPROV hProv = 0;
		HCRYPTHASH hHash = 0;
		BYTE hash[16] = {0};
		DWORD hashLen = 16;
		std::string result;
		if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
			if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
				CryptHashData(hHash, reinterpret_cast<const BYTE*>(data.data()), static_cast<DWORD>(data.size()), 0);
				if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
					char buf[33] = {0};
					for (int i = 0; i < 16; ++i)
						sprintf_s(buf + i * 2, 3, "%02x", hash[i]);
					result = buf;
				}
				CryptDestroyHash(hHash);
			}
			CryptReleaseContext(hProv, 0);
		}
		return result;
	}


	static std::string ToString(const char* msg) {
		return (msg && strlen(msg) > 0) ? std::string(msg) : std::string();
	}
	static std::string ToString(const std::string& msg) {
		return msg;
	}
};

