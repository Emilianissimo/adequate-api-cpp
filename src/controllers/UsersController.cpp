#include "controllers/UsersController.h"
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "core/errors/Errors.h"
#include <nlohmann/json.hpp>
#include "core/loggers/LoggerSingleton.h"

net::awaitable<Outcome> UsersController::index(Request& request) {
    std::ostringstream ss;
    ss << "UsersController::index: called, "
       << "method=" << request.method()
       << " target=" << request.target();

    LoggerSingleton::get().info(ss.str());
    ss.clear();

    std::size_t limit  = 50;
    std::size_t offset = 0;

    std::vector<UserSerializer> users = co_await service_.list(limit, offset);

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

net::awaitable<Outcome> UsersController::store(Request& request) {
    std::ostringstream ss;
    ss << "UsersController::store: called, "
       << "method=" << request.method()
       << " target=" << request.target();
    LoggerSingleton::get().info(ss.str());
    ss.clear();

    nlohmann::json body = request.json();
    UserCreateSerializer serializer;
    std::optional<std::string> error_msg;

    //TODO: Clean password value from json to log
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
        "UsersController::store: Checking up if any error occured. Error msg: " + (error_msg.has_value() ? *error_msg : "")
    );
    ss.clear();
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
