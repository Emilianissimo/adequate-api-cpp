#pragma once
#include <iostream>
#include "core/filters/BaseFilter.h"

class UserListFilter final : public BaseFilter {
public:
    std::optional<std::int64_t> id;
    std::optional<std::vector<std::int64_t>> id__in;
    std::optional<std::string> username;
    std::optional<std::vector<std::string>> username__in;
    std::optional<std::string> email;
    std::optional<std::size_t> limit;
    std::optional<std::size_t> offset;

    void parseRequestQuery(std::unordered_map<std::string, std::string> query) override {
        if (query.contains("limit")) {
            this->limit = std::stoull(query["limit"]);
        }
        if (query.contains("offset")) {
            this->offset = std::stoull(query["offset"]);
        }
        if (query.contains("username__in")) {
            this->username__in = this->parseStrings(query["username__in"]);
        }
        if (query.contains("username")) {
            this->username = query["username"];
        }
        if (query.contains("email")) {
            this->username = query["email"];
        }
        if (query.contains("id__in")) {
            this->id__in = this->parseIds(query["id__in"]);
        }
        if (query.contains("id")) {
            this->id = std::stoull(query["id"]);
        }
    };
};
