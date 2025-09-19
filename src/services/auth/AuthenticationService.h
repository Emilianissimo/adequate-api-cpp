#pragma once
#ifndef BEAST_API_AUTHENTICATIONSERVICE_H
#define BEAST_API_AUTHENTICATIONSERVICE_H
#include "repositories/users/UsersRepository.h"

class AuthenticationService {
public:
    explicit AuthenticationService(UsersRepository& usersRepository) : usersRepository_(usersRepository) {}
    net::awaitable<TokenResponseSerializer> obtainTokens(LoginSerializer& data) const;
    net::awaitable<void> registerUser(RegisterSerializer& data) const;
private:
    UsersRepository& usersRepository_;
};


#endif //BEAST_API_AUTHENTICATIONSERVICE_H