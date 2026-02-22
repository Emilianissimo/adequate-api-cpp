#pragma once
#include "core/filters/BaseFilter.h"

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
