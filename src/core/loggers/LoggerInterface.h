#pragma once
#include <string>

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class LoggerInterface {
public:
    virtual ~LoggerInterface() = default;

    virtual void log(LogLevel level, const std::string& msg) = 0;

    void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    void info (const std::string& msg) { log(LogLevel::INFO,  msg); }
    void warn (const std::string& msg) { log(LogLevel::WARN,  msg); }
    void error(const std::string& msg) { log(LogLevel::ERROR, msg); }
};
