#include "BaseFilter.h"
#include <vector>
#include <sstream>
#include <cstdint>

std::vector<std::int64_t> BaseFilter::parseIds(const std::string& input){
    std::vector<std::int64_t> out;
    std::istringstream ss(input);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) {
            try {
                out.push_back(std::stoull(token));
            } catch (...) { continue; }
        }
    }
    return out;
}

std::vector<std::string> BaseFilter::parseStrings(const std::string& input) {
    std::vector<std::string> out;
    std::istringstream ss(input);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) {
            try {
                out.push_back(token);
            } catch (...) { continue; }
        }
    }
    return out;
}
