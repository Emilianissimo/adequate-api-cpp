#include <chrono>
#include <sstream>
#include <iomanip>
#include <string>
#include <format> // C++20

inline std::chrono::system_clock::time_point
parse_pg_timestamp(const std::string& s) {
    std::tm tm{};
    std::istringstream iss(s);

    // формат без TZ: "2025-09-02 05:00:00"
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (iss.fail()) {
        throw std::runtime_error("Failed to parse timestamp: " + s);
    }

    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return tp;
}

std::string to_iso_string(const std::chrono::system_clock::time_point& tp) {
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&t); // UTC
    return std::format("{:%Y-%m-%dT%H:%M:%SZ}", tm);
}
