#pragma once
#include <string>
#include <string_view>

// 提取文件名的辅助函数
constexpr std::string_view ExtractFileName(std::string_view path) {
	size_t pos = path.find_last_of("/\\");
	return (pos == std::string_view::npos) ? path : path.substr(pos + 1);
}
#define MODULE_INFO ("| "+ std::string(ExtractFileName(__FILE__))+ ":" + std::string(__func__) + ":" + std::to_string(__LINE__) + " - ")