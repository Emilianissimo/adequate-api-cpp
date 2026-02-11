#pragma once
#include "interfaces/FiltersInterface.h"

class BaseFilter : public FiltersInterface {
public:
    virtual void parseRequestQuery(std::unordered_map<std::string, std::string> query) = 0;

    static std::vector<std::int64_t> parseIds(const std::string& input);
    static std::vector<std::string> parseStrings(const std::string& input);
};
