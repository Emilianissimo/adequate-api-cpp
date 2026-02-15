#pragma once
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "services/users/UsersService.h"
#include "../core/http/interfaces/HttpInterface.h"
#include "core/file_system/FileSystemService.h"

class UsersController {
public:
    explicit UsersController(UsersService& service) : service_(service) {}
    [[nodiscard]] net::awaitable<Outcome> index(const Request& request) const;
    net::awaitable<Outcome> store(const Request& request) const;
    net::awaitable<Outcome> update(const Request& request) const;

private:
    UsersService& service_;
};
