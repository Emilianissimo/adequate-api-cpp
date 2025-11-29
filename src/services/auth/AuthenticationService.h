#pragma once
#ifndef BEAST_API_AUTHENTICATIONSERVICE_H
#define BEAST_API_AUTHENTICATIONSERVICE_H
#include "repositories/users/UsersRepository.h"
#include "serializers/auth/LoginSerializer.h"
#include "serializers/auth/RegisterSerializer.h"
#include "serializers/auth/TokenResponseSerializer.h"
#include "services/jwt/JwtService.h"

class AuthenticationService {
public:
    explicit AuthenticationService(
        UsersRepository& usersRepository,
        JwtService& jwtService
    ) : usersRepository_(usersRepository), jwtService_(jwtService) {}
    net::awaitable<TokenResponseSerializer> obtainTokens(LoginSerializer& data) const;
    net::awaitable<void> registerUser(RegisterSerializer& data) const;
private:
    UsersRepository& usersRepository_;
    JwtService& jwtService_;
};


#endif //BEAST_API_AUTHENTICATIONSERVICE_H