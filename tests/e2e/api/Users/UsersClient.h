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
        UsersClient(std::string host, std::string port, const std::string& bearer = {})
        : client_(std::move(host), std::move(port))
        {
            if (!bearer.empty())
                client_.setDefaultHeader("Authorization",  bearer);
        }

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
            out.body = json::parse(response.body);
            return out;
        }

        UsersStoreResponse store(const json& payload) {
            auto res = client_.postJson("/users", payload.dump());

            UsersStoreResponse out;
            out.status = res.status;
            out.rawBody = res.body;
            out.body = json::parse(res.body);
            return out;
        }

        RawResponse patchPictureMultipart(
            const int64_t id,
            const std::string& boundary,
            const std::string& multipartBody
        )
        {
            const std::string target = "/users/" + std::to_string(id);
            return client_.patchMultipart(target, boundary, multipartBody);
        }

        HttpResponse remove(const int64_t id)
        {
            const std::string target = "/users/" + std::to_string(id);
            return client_.del(target);
        }

        RawResponse getRaw(
            const std::string& path,
            const std::vector<std::pair<std::string,std::string>>& extraHeaders = {}
        )
        {
            return client_.getRaw(path, extraHeaders);
        }
    private:
        TestHttpClient client_;
    };
}

#endif //BEAST_API_USERSCLIENT_H
