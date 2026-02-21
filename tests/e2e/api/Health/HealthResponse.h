//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_HEALTHRESPONSE_H
#define BEAST_API_HEALTHRESPONSE_H

#include <string>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>


namespace test::http
{
    namespace http = beast::http;

    struct HealthResponse
    {
        http::status statusCode{};
        std::string rawBody;
        std::string status;

        [[nodiscard]] bool isOk() const
        {
            return statusCode == http::status::ok;
        }

        static HealthResponse from(const http::status code, const std::string& body)
        {
            HealthResponse res;
            res.statusCode = code;
            res.rawBody = body;

            if (!body.empty())
            {
                auto json = nlohmann::json::parse(body);
                if (json.contains("status"))
                    res.status = json["status"].get<std::string>();
            }

            return res;
        }
    };
}

#endif //BEAST_API_HEALTHRESPONSE_H
