//
// Created by Emil Erofeevskiy on 30/11/25.
//

#include "AuthenticationMiddleware.h"

net::awaitable<Outcome> AuthenticationMiddleware::handle(Request& request, Next next) {
    auto& rawRequest = request.raw();
    std::optional<std::string> error_msg;

    if (auto header = rawRequest.base().find("Authorization"); header == rawRequest.base().end()) {
        LoggerSingleton::get().warn("AuthenticationMiddleware::handle: No header provided");
        JsonResult error_response{
            json{{"error", "Missing Authorization header"}},
            http::status::unauthorized,
        };
        co_return error_response;
    }

    std::string_view auth = rawRequest.base().at("Authorization");
    if (!auth.starts_with("Bearer ")) {
        LoggerSingleton::get().warn("AuthenticationMiddleware::handle: Incorrect header provided");
        JsonResult error_response{
            json{{"error", "Invalid Authorization format"}},
            http::status::unauthorized,
        };
        co_return error_response;
    }

    std::string_view token = auth.substr(7);


    UserEntity incomingUser;
    try {
        incomingUser = co_await this->jwtService_.decode(std::string(token));
    } catch (const JwtException& e) {
        error_msg = AuthenticationMiddleware::mapJwtError(e);
        LoggerSingleton::get().warn("AuthenticationMiddleware::handle: Jwt decode error");
    }
    if (error_msg) {
        JsonResult error_response(
            json{{"error", *error_msg}},
            http::status::unauthorized
        );
        co_return  error_response;
    }

    bool userExists;
    UserFilter filters;
    filters.id = incomingUser.id;
    try {
        userExists = co_await this->usersService_.exists(filters);
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn("AuthenticationMiddleware::handle: User not found");
    }

    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::unauthorized,
        };
        co_return error_response;
    }

    if (!userExists) {
        JsonResult error_response{
            json{{"error", "Your token is shit"}},
            http::status::unauthorized
        };
        co_return error_response;
    }

    request.user_id = incomingUser.id;

    co_return co_await next(request);
}