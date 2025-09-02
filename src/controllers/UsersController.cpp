#include "controllers/UsersController.h"
#include "core/http/ResponseTypes.h"
#include <nlohmann/json.hpp>

net::awaitable<Outcome> UsersController::index(Request& request) {
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
