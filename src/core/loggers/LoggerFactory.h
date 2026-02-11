#pragma once
#include "interfaces/LoggerInterface.h"
#include "core/loggers/strategies/ConsoleLoggerStrategy.h"
#include <memory>
#include <string>

class LoggerFactory {
public:
    static std::shared_ptr<LoggerInterface> create(const std::string& type) {
        if (type == "console") {
            return std::make_shared<ConsoleLoggerStrategy>();
        }
        // later maybe extended with "file", "syslog", "noop", etc.
        throw std::runtime_error("Unknown logger type: " + type);
    }
};
