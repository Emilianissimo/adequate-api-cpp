#pragma once
#include "core/http/ResponseTypes.h"
#include "core/request/Request.h"
#include "core/interfaces/HttpInterface.h"
#include <functional>

struct MiddlewareInterface {
    using Next = std::function<net::awaitable<Outcome>(Request&)>;

    virtual ~MiddlewareInterface() = default;
    virtual net::awaitable<Outcome> handle(Request& request, Next next) = 0;
    virtual net::awaitable<Response> after(const Request& request, Response&& response) {
        co_return std::move(response);
    }
};
