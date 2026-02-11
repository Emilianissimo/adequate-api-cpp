//
// Created by Emil Erofeevskiy on 30/11/25.
//

#pragma once
#ifndef BEAST_API_AUTHENTICATIONMIDDLEWARE_H
#define BEAST_API_AUTHENTICATIONMIDDLEWARE_H
#include "core/interfaces/MiddlewareInterface.h"
#include "../core/http/interfaces/HttpInterface.h"
#include "services/jwt/JwtService.h"
#include "services/users/UsersService.h"


class AuthenticationMiddleware : public MiddlewareInterface {
public:
    AuthenticationMiddleware(
        JwtService& jwtService,
        UsersService& usersService
    ) : jwtService_(jwtService), usersService_(usersService)
    {}

    net::awaitable<Outcome> handle(Request& request, Next next) override;

protected:
    JwtService& jwtService_;
    UsersService& usersService_;

    static std::string mapJwtError(const JwtException& e) {
        using enum JwtError;
        switch (e.code()) {
            case TokenExpired:
                return "Token expired";
            case InvalidSignature:
                return "Invalid signature";
            case InvalidClaim:
                return "Invalid claim";
            default:
                return "Your token is shit";
                // return "Invalid token";
        }
    }
};


#endif //BEAST_API_AUTHENTICATIONMIDDLEWARE_H