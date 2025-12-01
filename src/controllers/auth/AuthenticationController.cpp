//
// Created by Emil Erofeevskiy on 20/09/25.
//

#include <nlohmann/json.hpp>
#include "controllers/auth/AuthenticationController.h"
#include "serializers/auth/RegisterSerializer.h"
#include "serializers/auth/LoginSerializer.h"
#include "core/http/ResponseTypes.h"

net::awaitable<Outcome> AuthenticationController::registration(Request& request) const {
    LoggerSingleton::get().debug("AuthenticationController::registration: called", {
        {"method", request.method()},
        {"target", request.target()}
    });

    nlohmann::json body = request.json();
    RegisterSerializer serializer;
    std::optional<std::string> error_msg;

    LoggerSingleton::get().debug("AuthenticationController::registration: Validating data by serializer");
    try {
        serializer = serializer.from_json(body);
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn("AuthenticationController::registration: Validation failed, error: " + (error_msg.has_value() ? *error_msg : ""));
    }

    LoggerSingleton::get().debug("AuthenticationController::registration: Checking up if any error occurred. Error msg: " + (error_msg.has_value() ? *error_msg : ""));
    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::unprocessable_entity,
            request.keep_alive()
        };
        co_return error_response;
    }

    try {
        co_await service_.registerUser(serializer);
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn("AuthenticationController::registration: Validation failed, error: " + (error_msg.has_value() ? *error_msg : ""));
    }

    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::unprocessable_entity,
            request.keep_alive()
        };
        co_return error_response;
    }

    JsonResult response{
        json{},
        http::status::created,
        request.keep_alive()
    };
    co_return response;
}

net::awaitable<Outcome> AuthenticationController::login(Request& request) const {
    LoggerSingleton::get().debug("AuthenticationController::login: called", {
        {"method", request.method()},
        {"target", request.target()}
    });

    nlohmann::json body = request.json();
    LoginSerializer serializer;
    std::optional<std::string> error_msg;

    LoggerSingleton::get().debug("AuthenticationController::login: Validating data by serializer");
    try {
        serializer = serializer.from_json(body);
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn("AuthenticationController::login: Validation failed, error: " + (error_msg.has_value() ? *error_msg : ""));
    }

    LoggerSingleton::get().debug("AuthenticationController::login: Checking up if any error occurred. Error msg: " + (error_msg.has_value() ? *error_msg : ""));
    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::unprocessable_entity,
            request.keep_alive()
        };
        co_return error_response;
    }

    bool userExists;
    UserFilter filters;
    filters.email = serializer.email;
    try {
        userExists = co_await this->usersService_.exists(filters);
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn("AuthenticationController::login: Validation failed, error: " + (error_msg.has_value() ? *error_msg : ""));
    }

    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::bad_gateway,
            request.keep_alive()
        };
        co_return error_response;
    }

    if (!userExists) {
        JsonResult error_response{
            json{{"error", "Incorrect login or password"}},
            http::status::unprocessable_entity,
            request.keep_alive()
        };
        co_return error_response;
    }

    TokenResponseSerializer responseSerializer;
    try {
        responseSerializer = co_await this->service_.obtainTokens(serializer);
    } catch (const ValidationError& e) {
        error_msg = e.what();
        if (error_msg == "user_not_found") error_msg = "Incorrect credentials";
        LoggerSingleton::get().warn("AuthenticationController::login: Validation failed, error: " + (error_msg.has_value() ? *error_msg : ""));
    }

    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::unprocessable_entity,
            request.keep_alive()
        };
        co_return error_response;
    }

    LoggerSingleton::get().debug("Converting serializer to JSON");

    co_return JsonResult{
        responseSerializer.to_json(),
        http::status::created,
        request.keep_alive()
    };
}
