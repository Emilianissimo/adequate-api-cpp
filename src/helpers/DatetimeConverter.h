#pragma once
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>

using time_point = std::chrono::system_clock::time_point;

/// Parse PostgresSQL timestamp (with optional fractional seconds) into chrono::time_point (UTC)
inline time_point parse_pg_timestamp(const std::string& s) {
    std::tm tm{};
    int micros = 0;

    std::istringstream iss(s);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (iss.fail()) {
        throw std::runtime_error("Failed to parse timestamp: " + s);
    }

    // fractional seconds, e.g.123456
    if (iss.peek() == '.') {
        iss.get();

        std::string frac;
        while (std::isdigit(iss.peek())) {
            frac.push_back(static_cast<char>(iss.get()));
        }

        if (!frac.empty()) {
            if (frac.size() > 6) frac.resize(6);          // truncate to microseconds
            while (frac.size() < 6) frac.push_back('0');  // pad to 6 digits

            micros = std::stoi(frac);
        }
    }

    // interpret as UTC (not localtime!)
#if defined(_WIN32)
    const std::time_t tt = _mkgmtime(&tm);
#else
    const std::time_t tt = timegm(&tm); // GNU extension
#endif
    auto tp = std::chrono::system_clock::from_time_t(tt);
    tp += std::chrono::microseconds(micros);
    return tp;
}

/// Convert chrono::time_point to ISO-8601 string in UTC (e.g. 2025-09-04T18:32:53.123Z)
inline std::string to_iso_string(const time_point& tp) {
    using namespace std::chrono;

    const auto s = time_point_cast<seconds>(tp);
    const auto rep = duration_cast<microseconds>(tp - s).count();

    std::time_t t = system_clock::to_time_t(s);
    std::tm gmt{};
#if defined(_WIN32)
    gmtime_s(&gmt, &t);
#else
    gmtime_r(&t, &gmt);
#endif

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &gmt);

    std::ostringstream oss;
    oss << buf;
    if (rep > 0) {
        oss << "." << std::setfill('0') << std::setw(6) << rep;
    }
    oss << "Z";
    return oss.str();
}
