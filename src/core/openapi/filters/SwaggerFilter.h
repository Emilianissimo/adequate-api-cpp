//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_SWAGGERFILTER_H
#define BEAST_API_SWAGGERFILTER_H

#include <optional>

#include "core/filters/BaseFilter.h"

class SwaggerFilter final : public BaseFilter {
public:
    std::optional<bool> json;

    void parseRequestQuery(std::unordered_map<std::string, std::string> query) override {
        if (query.contains("json")) {
            this->json = true;
        }
    };
};

#endif //BEAST_API_SWAGGERFILTER_H
