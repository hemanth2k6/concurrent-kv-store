#include "redis_lite/logger.hpp"
#include <ctime>
#include <iomanip>

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::log(LogLevel level, const std::string &message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "]";
    switch (level)
    {
    case LogLevel::INFO:
        oss << "[INFO]";
        break;
    case LogLevel::ERROR:
        oss << "[ERROR]";
        break;
    }
    oss << message;
    std::cout << oss.str() << std::endl;
}