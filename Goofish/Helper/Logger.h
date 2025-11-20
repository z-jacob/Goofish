#pragma once
#include <string>
#include <functional>

#include "../Helper/Utils.h"

inline std::string ToString(const char* msg) {
	return (msg && strlen(msg) > 0) ? std::string(msg) : std::string();
}
inline std::string ToString(const std::string& msg) {
	return msg;
}

#define LOG_INFO(prefix, message) Logger::Log(Logger::Level::Info, (ToString(message).length() > 0 ? (prefix + " -> " + ToString(message)) : prefix))
#define LOG_WARNING(prefix, message) Logger::Log(Logger::Level::Warning, (ToString(message).length() > 0 ? (prefix + " -> " + ToString(message)) : prefix))
#define LOG_ERROR(prefix, message) Logger::Log(Logger::Level::Error, (ToString(message).length() > 0 ? (prefix + " -> " + ToString(message)) : prefix))

class Logger
{
public:
	enum class Level { Info, Warning, Error };
	using Callback = std::function<void(const Level level,const std::string& formattedMessage)>;

	// 可选提供文件路径以写入日志文件；可多次调用但只有第一次打开文件有效
	static void Init(const std::string& filePath = "");
	static void SetCallback(Callback cb);

	// 主调用接口
	static void Log(Level level, const std::string& message);

	// 返回格式化后的字符串（便于 UI 直接显示）
	static std::string Format(Level level, const std::string& message);

private:
	Logger() = delete;
	~Logger() = delete;
};