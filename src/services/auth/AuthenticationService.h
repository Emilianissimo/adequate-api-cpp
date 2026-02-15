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
        JwtService& jwtService,
        net::thread_pool& blockingPool
    ) : usersRepository_(usersRepository), jwtService_(jwtService), blockingPool_(blockingPool) {}
    net::awaitable<TokenResponseSerializer> obtainTokens(LoginSerializer& data) const;
    net::awaitable<void> registerUser(RegisterSerializer& data) const;
private:
    UsersRepository& usersRepository_;
    JwtService& jwtService_;
    net::thread_pool& blockingPool_;
};


#endif //BEAST_API_AUTHENTICATIONSERVICE_H