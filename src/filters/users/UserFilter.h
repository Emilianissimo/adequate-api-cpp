#pragma once

#include <optional>

#include "core/filters/BaseFilter.h"
#include "core/openapi/OpenApiParamBuilder.h"

class UserFilter final : public BaseFilter {
public:
    std::optional<std::int64_t> id;
    std::optional<std::string> username;
    std::optional<std::string> email;

    void parseRequestQuery(std::unordered_map<std::string, std::string> query) override {
        if (query.contains("username")) {
            this->username = query["username"];
        }
        if (query.contains("email")) {
            this->email = query["email"];
        }
        if (query.contains("id")) {
            this->id = std::stoull(query["id"]);
        }
    };
};

/// OpenApiGenerator Template

template <class FilterT>
struct OpenApiFilterSpec;

template <>
struct OpenApiFilterSpec<UserFilter> {
    static nlohmann::json parameters() {
        nlohmann::json p = nlohmann::json::array();

        p.push_back(OpenApiParamBuilder::query(
            "id", {{"type","integer"},{"format","int64"}})
        );
        p.push_back(OpenApiParamBuilder::query(
            "username", {{"type","string"}})
        );
        p.push_back(OpenApiParamBuilder::query(
            "email", {{"type","string"},{"format","email"}})
        );

        return p;
    }
};
