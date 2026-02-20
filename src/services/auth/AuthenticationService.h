#ifndef BEAST_API_AUTHENTICATIONSERVICE_H
#define BEAST_API_AUTHENTICATIONSERVICE_H
#include "core/hashers/SodiumPasswordHasher.h"
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
        net::thread_pool& blockingPool,
        const std::shared_ptr<app::security::SodiumPasswordHasher>& passwordHasher
    ) :
    usersRepository_(usersRepository),
    jwtService_(jwtService),
    blockingPool_(blockingPool),
    passwordHasher_(passwordHasher) {}
    net::awaitable<TokenResponseSerializer> obtainTokens(LoginSerializer data) const;
    net::awaitable<void> registerUser(RegisterSerializer data) const;
private:
    UsersRepository& usersRepository_;
    JwtService& jwtService_;
    net::thread_pool& blockingPool_;
    std::shared_ptr<app::security::SodiumPasswordHasher> passwordHasher_;
};


#endif //BEAST_API_AUTHENTICATIONSERVICE_H