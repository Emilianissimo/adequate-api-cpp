#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

class FiltersInterface {
public:
    virtual ~FiltersInterface() = default;

protected:
    virtual void parseRequestQuery(std::unordered_map<std::string, std::string> query) {}
};
