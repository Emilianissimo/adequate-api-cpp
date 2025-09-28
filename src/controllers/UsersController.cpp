#include "controllers/UsersController.h"
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "core/errors/Errors.h"
#include <nlohmann/json.hpp>
#include "core/loggers/LoggerSingleton.h"
#include "filters/users/UserListFilter.h"

net::awaitable<Outcome> UsersController::index(Request& request) const {
    LoggerSingleton::get().debug("UsersController::index: called", {
        {"method", request.method()},
        {"target", request.target()}
    });

    UserListFilter filters;
    filters.parseRequestQuery(request.query());
    filters.limit = std::clamp<std::size_t>(filters.limit.value_or(50), 1, 1000);

    std::vector<UserSerializer> users = co_await service_.list(filters);

    LoggerSingleton::get().debug("Converting serializer to JSON");
    nlohmann::json body = nlohmann::json::array();
    for (auto& user : users) {
        body.push_back(user.to_json());
    }

    JsonResult result{
        body,
        http::status::ok,
        request.keep_alive()
    };
    co_return result;
}

net::awaitable<Outcome> UsersController::store(Request& request) const {
    LoggerSingleton::get().debug("UsersController::store: called", {
        {"method", request.method()},
        {"target", request.target()}
    });

    nlohmann::json body = request.json();
    UserCreateSerializer serializer;
    std::optional<std::string> error_msg;

    // TODO: Clean password value from json to log
    LoggerSingleton::get().debug(
        "UsersController::store: Validating data by serializer. Data: " + body.dump()
    );
    try {
        serializer = serializer.from_json(body);
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn(
            "UsersController::store: Validation failed, error: " + (error_msg.has_value() ? *error_msg : "")
        );
    }

    LoggerSingleton::get().debug(
        "UsersController::store: Checking up if any error occurred. Error msg: " + (error_msg.has_value() ? *error_msg : "")
    );
    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::unprocessable_entity,
            request.keep_alive()
        };
        co_return error_response;
    }

    UserCreateResponseSerializer user;
    try {
        user = co_await service_.create(serializer);        
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn(
            "UsersController::store: User creation failed: " + (error_msg.has_value() ? *error_msg : "")
        );
    }

    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::conflict,
            request.keep_alive()
        };
        co_return error_response;
    }

    JsonResult response{
        user.to_json(),
        http::status::created,
        request.keep_alive()
    };
    co_return response;
}

net::awaitable<Outcome> UsersController::update(Request& request) const {
    std::string id_str = request.path_params.at("id");
    int id = std::stoi(id_str);

    LoggerSingleton::get().info("UsersController::update: called", {
        {"method", request.method()},
        {"target", request.target()}
    });

    nlohmann::json body = request.json();
    UserUpdateSerializer serializer;
    std::optional<std::string> error_msg;

    // TODO: Clean password value from json to log
    LoggerSingleton::get().debug(
        "UsersController::update: Validating data by serializer. Data: " + body.dump()
    );
    try {
        serializer = serializer.from_json(body);
        serializer.id = id;
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn(
            "UsersController::update: Validation failed, error: " + (error_msg.has_value() ? *error_msg : "")
        );
    }

    LoggerSingleton::get().debug(
        "UsersController::update: Checking up if any error occurred. Error msg: " + (error_msg.has_value() ? *error_msg : "")
    );
    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::unprocessable_entity,
            request.keep_alive()
        };
        co_return error_response;
    }

    UserCreateResponseSerializer user;
    try {
        co_await service_.update(serializer);
    } catch (const ValidationError& e) {
        error_msg = e.what();
        LoggerSingleton::get().warn(
            "UsersController::update: User creation failed: " + (error_msg.has_value() ? *error_msg : "")
        );
    }

    if (error_msg){
        JsonResult error_response{
            json{{"error", *error_msg}},
            http::status::conflict,
            request.keep_alive()
        };
        co_return error_response;
    }

    JsonResult response{
        json{},
        http::status::no_content,
        request.keep_alive()
    };
    co_return response;
}
