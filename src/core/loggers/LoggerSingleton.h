#pragma once
#include <memory>
#include "core/loggers/LoggerInterface.h"

class LoggerSingleton {
public:
    static void init(std::shared_ptr<LoggerInterface> logger) {
        instance_ = std::move(logger);
    }

    static LoggerInterface& get() {
        return *instance_;
    }

private:
    inline static std::shared_ptr<LoggerInterface> instance_;
};
