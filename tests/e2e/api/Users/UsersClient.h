//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_USERSCLIENT_H
#define BEAST_API_USERSCLIENT_H

#include "../../base/TestHttpClient.h"
#include "UsersResponse.h"
#include <nlohmann/json.hpp>
#include <string>

namespace test::http
{
    namespace http = beast::http;

    class UsersClient
    {
    public:
        UsersClient(std::string host, std::string port)
        : client_(std::move(host), std::move(port)) {}

        UsersIndexResponse index(const std::string& query = "")
        {
            std::string target = "/users";
            if (!query.empty())
            {
                target += (query.front() == '?' ? "" : "?");
                target += query;
            }

            auto response = client_.get(target);

            UsersIndexResponse out;
            out.status = response.status;
            out.rawBody = response.body;
            out.body = nlohmann::json::parse(response.body);
            return out;
        }

        UsersStoreResponse store(const nlohmann::json& payload) {
            auto res = client_.postJson("/users", payload.dump());

            UsersStoreResponse out;
            out.status = res.status;
            out.rawBody = res.body;
            out.body = nlohmann::json::parse(res.body);
            return out;
        }
    private:
        TestHttpClient client_;
    };
}

#endif //BEAST_API_USERSCLIENT_H
