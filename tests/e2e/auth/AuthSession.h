//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_AUTHSESSION_H
#define BEAST_API_AUTHSESSION_H

#include "AuthClient.h"
#include <string>
#include <stdexcept>

namespace test::http
{
    struct AuthSession
    {
        std::string bearer;

        static AuthSession obtain(std::string host, std::string port) {
            AuthClient auth(std::move(host), std::move(port));

            const std::string email = "e2e_" + std::to_string(::getpid()) + "@example.com";
            const std::string username = "e2e_" + std::to_string(::getpid());
            const std::string password = "Passw0rd!";

            if (auto [status, rawBody] = auth.registerUser(email, username, password); !(status == http::status::created ||
                  status == http::status::unprocessable_entity ||
                  status == http::status::conflict))
            {
                throw std::runtime_error(
                   "registration failed with status=" + std::to_string(static_cast<int>(status)) +
                   " body=" + rawBody
               );
            }

            const auto login = auth.login(email, password);
            if (!login.ok()) {
                throw std::runtime_error("login failed: " + login.rawBody);
            }

            return AuthSession{ "Bearer " + login.accessToken };
        }
    };
}

#endif //BEAST_API_AUTHSESSION_H
