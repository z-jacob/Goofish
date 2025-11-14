#include "Logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <fstream>

namespace {
    std::mutex s_logMutex;
    std::ofstream s_logOfs;
    Logger::Callback s_logCallback;
}

void Logger::Init(const std::string& filePath)
{
    std::lock_guard<std::mutex> lk(s_logMutex);
    if (!filePath.empty() && !s_logOfs.is_open()) {
        s_logOfs.open(filePath, std::ios::app | std::ios::out);
    }
}

void Logger::SetCallback(Callback cb)
{
    std::lock_guard<std::mutex> lk(s_logMutex);
    s_logCallback = std::move(cb);
}

std::string Logger::Format(Level level, const std::string& message)
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    std::tm tm;
#if defined(_MSC_VER)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000;

    const char* levelStr = "INFO";
    switch (level) {
    case Level::Info: levelStr = "INFO"; break;
    case Level::Warning: levelStr = "WARNING"; break;
    case Level::Error: levelStr = "ERROR"; break;
    }

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << "." << std::setw(3) << std::setfill('0') << ms
        << " | " << std::left << std::setw(10) << std::setfill(' ') << levelStr
        << " " << message;
    return oss.str();
}

void Logger::Log(Level level, const std::string& message)
{
    std::string formatted = Format(level, message);

    {
        std::lock_guard<std::mutex> lk(s_logMutex);
        if (s_logOfs.is_open()) {
            s_logOfs << formatted << std::endl;
            s_logOfs.flush();
        }
        if (s_logCallback) {
            // 回调由调用者负责线程安全（若来自非 UI 线程，请通过 PostMessage/SendMessage 传回主线程）
            s_logCallback(formatted);
        }
    }
}