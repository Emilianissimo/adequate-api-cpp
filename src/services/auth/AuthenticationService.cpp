#include "AuthenticationService.h"

#include "core/helpers/Offload.h"

net::awaitable<TokenResponseSerializer> AuthenticationService::obtainTokens(LoginSerializer data) const {
    LoggerSingleton::get().info("AuthenticationService::obtainTokens: called", {
        {"email", data.email},
        {"password", data.password},
    });

    UserFilter filters;
    filters.email = data.email;

    const UserEntity user = co_await this->usersRepository_.getOne(filters);

    LoggerSingleton::get().debug("AuthenticationService::obtainTokens: checking password");

    auto plain = std::make_shared<std::string>(data.password);
    auto stored = std::make_shared<std::string>(user.password.value());
    auto hasher = passwordHasher_; // copy shared_ptr

    auto [ok, needsRehash] = co_await async_offload(
        blockingPool_.get_executor(),
        [plain, stored, hasher]() {
            bool nr = false;
            bool res = hasher->verify(*plain, *stored, &nr);
            return std::pair{res, nr};
        }
    );
    if (!ok) throw ValidationError("Incorrect credentials");
    if (needsRehash) {
        // update hash to new, harder one
        // auto newHash = co_await net::co_spawn(
        //     blockingPool_.get_executor(),
        //     [plain, &hasher = passwordHasher_]() -> net::awaitable<std::string> {
        //         co_return hasher.hash(plain);
        //     },
        //     boost::asio::use_awaitable
        // );
        // user.password = newHash; + persist
    }


    TokenResponseSerializer tokenPair;
    // TODO: add refresh token logic (I'm lazy as fuck)
    tokenPair.refreshToken = "None";
    tokenPair.accessToken = co_await this->jwtService_.encode(user);
    co_return tokenPair;
}

net::awaitable<void> AuthenticationService::registerUser(RegisterSerializer data) const {
    LoggerSingleton::get().info("AuthenticationService::registerUser: called", {
        {"email", data.email},
        {"username", data.username}
    });
    UserEntity user = data.toEntity();
    auto pwdPtr = std::make_shared<std::string>(user.password.value());
    auto hasher = passwordHasher_; // copy shared_ptr
    auto hash = co_await async_offload(
        blockingPool_.get_executor(),
        [pwdPtr, hasher]() {
            return hasher->hash(*pwdPtr);
        }
    );
    user.password = std::move(hash);

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
