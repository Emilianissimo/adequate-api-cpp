#pragma once
#include <string>

enum class LogLevel { DEBUG, INFO, WARN, ERR };

class LoggerInterface {
public:
    virtual ~LoggerInterface() = default;

    virtual void log(LogLevel level, const std::string& msg, const std::optional<std::map<std::string, std::any>>& params) = 0;

    void info (const std::string& msg, const std::map<std::string, std::any>& params = {})
    {
        if (params.empty())
        {
            log(LogLevel::INFO, msg, std::nullopt);
            return;
        }
        log(LogLevel::INFO, msg, params);
    }
    void debug(const std::string& msg, const std::map<std::string, std::any>& params = {})
    {
        if (params.empty())
        {
            log(LogLevel::DEBUG, msg, std::nullopt);
            return;
        }
        log(LogLevel::DEBUG, msg, params);
    }
    void warn (const std::string& msg, const std::map<std::string, std::any>& params = {})
    {
        if (params.empty())
        {
            log(LogLevel::WARN, msg, std::nullopt);
            return;
        }
        log(LogLevel::WARN, msg, params);
    }
    void error(const std::string& msg, const std::map<std::string, std::any>& params = {})
    {
        if (params.empty())
        {
            log(LogLevel::ERR, msg, std::nullopt);
            return;
        }
        log(LogLevel::ERR, msg, params);
    }
};
