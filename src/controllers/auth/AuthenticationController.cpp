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

    try {
        // UsersSerivice::exists
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn("AuthenticationController::login: Validation failed, error: " + (error_msg.has_value() ? *error_msg : ""));
    }
}
