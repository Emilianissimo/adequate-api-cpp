//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_HEALTHCHECKCLIENT_H
#define BEAST_API_HEALTHCHECKCLIENT_H

#include "../../base/TestHTTPClient.h"
#include "HealthResponse.h"

namespace test::http
{
    class HealthCheckClient
    {
    public:
        HealthCheckClient(std::string host, std::string port): client_(std::move(host), std::move(port)) {}

        HealthResponse check()
        {
            auto [status, body] = client_.get("/health");

            return HealthResponse::from(status, body);
        }

    private:
        TestHttpClient client_;
    };
}

#endif //BEAST_API_HEALTHCHECKCLIENT_H
