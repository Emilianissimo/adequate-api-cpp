#include <iostream>
#include "core/interfaces/FiltersInterface.h"

class BaseFilter : public FiltersInterface {
public:
    virtual void parseRequestQuery(std::unordered_map<std::string, std::string> query) = 0;
    std::vector<std::uint64_t> parseIds(const std::string& input);
    std::vector<std::string> parseStrings(const std::string& input);
};
