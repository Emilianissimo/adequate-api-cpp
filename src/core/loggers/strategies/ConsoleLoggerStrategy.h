#pragma once
#include "core/loggers/LoggerInterface.h"
#include <iostream>
#include <chrono>
#include <iomanip>

class ConsoleLoggerStrategy : public LoggerInterface {
public:
    void log(LogLevel level, const std::string& msg) override {
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);

        std::cerr << "[" << std::put_time(&tm, "%F %T") << "] "
                  << levelToString(level) << ": " << msg << std::endl;
    }

private:
    static std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
        }
        return "?";
    }
};
