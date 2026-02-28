#include "controllers/HealthController.h"
using nlohmann::json;

net::awaitable<Outcome> HealthController::index(const Request& request) {
    JsonResult result{
        json{{"status","alive"}},
        http::status::ok,
        request.keep_alive()
    };
    co_return result;
}
