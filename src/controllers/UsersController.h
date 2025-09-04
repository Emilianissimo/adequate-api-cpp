#pragma once
#include "core/interfaces/HttpInterface.h"
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "services/users/UsersServiceInterface.h"
#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>

namespace net = boost::asio;

class UsersController {
public:
    explicit UsersController(UsersServiceInterface& service) : service_(service) {}
    net::awaitable<Outcome> index(Request& request);
    net::awaitable<Outcome> store(Request& reqeust);

private:
    UsersServiceInterface& service_;
};
