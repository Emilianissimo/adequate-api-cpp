#pragma once
#include "core/loggers/LoggerInterface.h"
#include "core/loggers/strategies/ConsoleStrategy.h"
#include <memory>
#include <string>

class LoggerFactory {
public:
    static std::shared_ptr<LoggerInterface> create(const std::string& type) {
        if (type == "console") {
            return std::make_shared<ConsoleLogger>();
        }
        // later maybe extended with "file", "syslog", "noop" and etc.
        throw std::runtime_error("Unknown logger type: " + type);
    }
};
