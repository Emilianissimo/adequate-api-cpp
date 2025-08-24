#pragma once
#include "core/interfaces/HttpInterface.h"
#include "core/http/ResponseTypes.h"
#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>

namespace net = boost::asio;

class HealthController {
public:
    net::awaitable<Outcome> index(Request& req);
};
