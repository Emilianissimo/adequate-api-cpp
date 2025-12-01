#include "AuthenticationService.h"
#include <bcrypt/BCrypt.hpp>

net::awaitable<TokenResponseSerializer> AuthenticationService::obtainTokens(LoginSerializer& data) const {
    LoggerSingleton::get().info("AuthenticationService::obtainTokens: called", {
        {"email", data.email},
        {"password", data.password},
    });

    UserFilter filters;
    filters.email = data.email;

    const UserEntity user = co_await this->usersRepository_.getOne(filters);

    LoggerSingleton::get().debug("AuthenticationService::obtainTokens: checking password");
    if (!BCrypt::validatePassword(data.password, user.password.value())) throw ValidationError("Incorrect credentials");

    TokenResponseSerializer tokenPair;
    // TODO: add refresh token logic (I'm lazy as fuck)
    tokenPair.refreshToken = "None";
    tokenPair.accessToken = co_await this->jwtService_.encode(user);
    co_return tokenPair;
}

net::awaitable<void> AuthenticationService::registerUser(RegisterSerializer& data) const {
    LoggerSingleton::get().info("AuthenticationService::registerUser: called", {
        {"email", data.email},
        {"password", data.password},
        {"username", data.username}
    });
    UserEntity user = data.toEntity();
    user.password = BCrypt::generateHash(user.password.value());

    LoggerSingleton::get().debug("AuthenticationService::registerUser: Creating user by repo");
    try {
        co_await usersRepository_.create(user);
        co_return;
    } catch (const DbError& e) {
        LoggerSingleton::get().warn(
            "AuthenticationService::registerUser: Error of creating user " + std::to_string(static_cast<int>(e.code())) +
            ", msg=" + e.what()
        );
        switch (e.code()) {
            case DbErrorCode::NotNullViolation:
                throw ValidationError("Missing required fields");
            default:
                throw; // Others are 500
        }
    }
}
