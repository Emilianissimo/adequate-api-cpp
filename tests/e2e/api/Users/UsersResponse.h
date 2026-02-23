//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_USERSRESPONSE_H
#define BEAST_API_USERSRESPONSE_H

#include <string>
#include <nlohmann/json.hpp>

using nlohmann::json;

namespace test::http
{
    namespace http = beast::http;

    struct UsersIndexResponse {
        http::status status{};
        json body; // array
        std::string rawBody;

        bool ok() const { return status == http::status::ok && body.is_array(); }
    };

    struct UsersStoreResponse {
        http::status status{};
        json body;
        std::string rawBody;

        bool created() const { return status == http::status::created; }
    };

}

#endif //BEAST_API_USERSRESPONSE_H
