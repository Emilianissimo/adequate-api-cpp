#pragma once
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>

using time_point = std::chrono::system_clock::time_point;

inline time_point parse_pg_timestamp(const std::string& s) {
    std::tm tm{};
    std::istringstream iss(s);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (iss.fail()) throw std::runtime_error("Failed to parse timestamp: " + s);
    // mktime интерпретирует tm как локальное; если хочешь UTC — используй timegm(3) или эквивалент.
    std::time_t tt = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(tt);
}

inline std::string to_iso_string(const time_point& tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm gmt{};
#if defined(_WIN32)
    gmtime_s(&gmt, &t);
#else
    gmt = *std::gmtime(&t);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &gmt);
    return std::string(buf);
}
