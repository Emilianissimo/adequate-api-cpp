#pragma once
#include "core/interfaces/FiltersInterface.h"

class BaseFilter : public FiltersInterface {
public:
    void parseRequestQuery(std::unordered_map<std::string, std::string> query) override = 0;
    std::vector<std::int64_t> parseIds(const std::string& input);
    std::vector<std::string> parseStrings(const std::string& input);
};
