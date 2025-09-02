#pragma once
#include "core/routers/Router.h"
#include "di/AppContext.h"

template <class C, class Method>
inline Router::RouteFn bind_handler(C* obj, Method m) {
    return [obj, m](Request& req) -> net::awaitable<Outcome> {
        co_return co_await (obj->*m)(req);
    };
}

namespace app {
    void define_routes(Router& router, const std::shared_ptr<AppContext> ctx);
}
