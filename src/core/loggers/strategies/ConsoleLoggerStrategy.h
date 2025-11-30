#pragma once
#include "../../interfaces/LoggerInterface.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <boost/algorithm/string/join.hpp>

class ConsoleLoggerStrategy final : public LoggerInterface {
public:
    void log(LogLevel level, const std::string& msg, const std::optional<std::map<std::string, std::any>>& params) override {
        const auto now = std::chrono::system_clock::now();
        const auto t   = std::chrono::system_clock::to_time_t(now);
        const std::tm tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << "[" << std::put_time(&tm, "%F %T") << "] " << levelToString(level) << ": " << msg;
        if (params && !params->empty()) {
            oss << " | Params: ";
            bool first = true;
            for (const auto& [k, v] : *params) {
                if (!first) oss << ", ";
                first = false;

                if (v.type() == typeid(std::string)) {
                    oss << k << '=' << std::any_cast<const std::string&>(v);
                } else if (v.type() == typeid(int)) {
                    oss << k << '=' << std::any_cast<int>(v);
                } else if (v.type() == typeid(int64_t)) {
                    oss << k << '=' << std::any_cast<int64_t>(v);
                } else if (v.type() == typeid(std::vector<std::string>)){
                    const auto& vec = std::any_cast<const std::vector<std::string>&>(v);
                    oss << k << "=" << boost::algorithm::join(vec, ",");
                } else if (v.type() == typeid(std::vector<int64_t>)){
                    const auto& vec = std::any_cast<const std::vector<int64_t>&>(v);
                    std::vector<std::string> strs;
                    strs.reserve(vec.size());
                    for (auto x : vec) strs.push_back(std::to_string(x));
                    oss << k << "=" << boost::algorithm::join(strs, ",");
                } else if (v.type() == typeid(std::vector<int>)){
                    const auto& vec = std::any_cast<const std::vector<int>&>(v);
                    std::vector<std::string> strs;
                    strs.reserve(vec.size());
                    for (auto x : vec) strs.push_back(std::to_string(x));
                    oss << k << "=" << boost::algorithm::join(strs, ",");
                } else {
                    oss << k << "=<unsupported>";
                }
            }
        }
        std::cerr << oss.str() << std::endl;
    }

private:
    static std::string levelToString(const LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERR: return "ERROR";
        }
        return "?";
    }
};
