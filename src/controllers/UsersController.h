#pragma once
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "services/users/UsersServiceInterface.h"
#include <boost/asio/awaitable.hpp>
#include "core/interfaces/HttpInterface.h"

class UsersController {
public:
    explicit UsersController(UsersServiceInterface& service) : service_(service) {}
    net::awaitable<Outcome> index(Request& request);
    net::awaitable<Outcome> store(Request& reqeust);
    net::awaitable<Outcome> update(Request& reqeust);

private:
    UsersServiceInterface& service_;
};
