#pragma once
#include "core/interfaces/HttpInterface.h"
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>

class HealthController {
public:
    net::awaitable<Outcome> index(Request& req);
};
