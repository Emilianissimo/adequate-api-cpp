#include "routes/Routes.h"
#include "controllers/HealthController.h"

template <class C, class Method>
inline Router::RouteFn bind_handler(C* obj, Method m) {
    return [obj, m](Request& req) -> net::awaitable<Outcome> {
        co_return co_await (obj->*m)(req);
    };
}

namespace app{
    static HealthController healthController;

    void define_routes(Router& router, const AppContext& ctx){
        // Middlewares
        // router.use(std::make_shared<RequestIdMiddleware>());
        // router.use(std::make_shared<LoggingMiddleware>());

        router.get("/health", bind_handler(&healthController, &HealthController::index));

        // DI example
        // r.get("/users", [repo = std::make_shared<UserRepo>(ctx.pg)]
        //     (Request& req) -> net::awaitable<Outcome> { ... });
    }
}
