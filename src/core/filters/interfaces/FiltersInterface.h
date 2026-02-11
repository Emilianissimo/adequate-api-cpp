#pragma once

#include <unordered_map>
#include <string>

class FiltersInterface {
public:
    virtual ~FiltersInterface() = default;

protected:
    virtual void parseRequestQuery(const std::unordered_map<std::string, std::string>& query) {}
};
