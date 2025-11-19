#include "Utils.h"

// 提取文件名的辅助函数

std::string_view Utils::ExtractFileName(std::string_view path) {
	size_t pos = path.find_last_of("/\\");
	return (pos == std::string_view::npos) ? path : path.substr(pos + 1);
}

float Utils::GetDpi()
{
	HDC hDC = ::GetDC(nullptr); // 获取整个屏幕的 DC
	int dpiX = ::GetDeviceCaps(hDC, LOGPIXELSX); // 水平 DPI
	int dpiY = ::GetDeviceCaps(hDC, LOGPIXELSY); // 垂直 DPI
	::ReleaseDC(nullptr, hDC);
	return dpiY / 96.0f;
}
