#pragma once
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "services/users/UsersService.h"
#include "core/interfaces/HttpInterface.h"

class UsersController {
public:
    explicit UsersController(UsersService& service) : service_(service) {}
    net::awaitable<Outcome> index(Request& request) const;
    net::awaitable<Outcome> store(Request& request) const;
    net::awaitable<Outcome> update(Request& request) const;

private:
    UsersService& service_;
};
