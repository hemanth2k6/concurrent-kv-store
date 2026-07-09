#pragma once
#include <iostream>
#include <sstream>
#include <mutex>
enum class LogLevel
{
    INFO,
    ERROR
};
class Logger
{
public:
    static Logger &instance();
    void log(LogLevel level, const std::string &message);

private:
    Logger() = default;
    std::mutex mutex_;
};
#define LOG_INFO(msg) Logger::instance().log(LogLevel::INFO, msg)
#define LOG_ERROR(msg) Logger::instance().log(LogLevel::ERROR, msg)
