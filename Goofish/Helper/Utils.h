#pragma once
#include <string>
#include <string_view>

#include <windows.h>


class Utils {
public:
	static float GetDpi();
	static std::string_view ExtractFileName(std::string_view path);

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
};

#define MODULE_INFO ("| "+ std::string(Utils::ExtractFileName(__FILE__))+ ":" + std::string(__func__) + ":" + std::to_string(__LINE__))

