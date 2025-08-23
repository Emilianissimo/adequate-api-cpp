#pragma once
#include "HttpInterface.h"
#include <functional>

using Next = std::function<net::awaitable<Response>(Request&)>;

struct MiddlewareInterface {
    virtual ~MiddlewareInterface() = default;
    virtual net::awaitable<Response> handle(Request& request, Next next) = 0;
};
