#pragma once
#include <string>
#include <string_view>

#include <windows.h>


class Utils {
public:
	static float GetDpi();
	static std::string_view ExtractFileName(std::string_view path);
};

#define MODULE_INFO ("| "+ std::string(Utils::ExtractFileName(__FILE__))+ ":" + std::string(__func__) + ":" + std::to_string(__LINE__))

