//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_AUTHCLIENT_H
#define BEAST_API_AUTHCLIENT_H

#include "../base/TestHttpClient.h"
#include <nlohmann/json.hpp>
#include <string>

namespace test::http
{
    namespace http = beast::http;
    using json = nlohmann::json;

    struct LoginResult {
        http::status status{};
        std::string accessToken;
        std::string rawBody;

        bool ok() const { return status == http::status::created && !accessToken.empty(); }
    };

    struct SimpleResponse {
        http::status status{};
        std::string rawBody;
    };

    class AuthClient
    {
    public:
        AuthClient(std::string host, std::string port)
            : client_(std::move(host), std::move(port)) {}

        SimpleResponse registerUser(const std::string& email,
                              const std::string& username,
                              const std::string& password)
        {
            const json payload{
                {"email", email},
                {"username", username},
                {"password", password}
            };

            const auto res = client_.postJson("/register", payload.dump());
            return {
                res.status,
                res.body
            };
        }

        LoginResult login(const std::string& email, const std::string& password)
        {
            const json payload{
                {"email", email},
                {"password", password}
            };

            auto res = client_.postJson("/login", payload.dump());

            LoginResult out;
            out.status = res.status;
            out.rawBody = res.body;

            auto j = json::parse(res.body);
            out.accessToken = j.value("accessToken", "");

            return out;
        }

    private:
        TestHttpClient client_;
    };
}

#endif //BEAST_API_AUTHCLIENT_H
