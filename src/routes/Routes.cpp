#include "routes/Routes.h"
#include "controllers/HealthController.h"
#include "controllers/UsersController.h"

namespace app{
    void define_routes(Router& router, const std::shared_ptr<AppContext> ctx){
        // Middlewares
        // router.use(std::make_shared<RequestIdMiddleware>());
        // router.use(std::make_shared<LoggingMiddleware>());

        router.get("/health", bind_handler(ctx->healthController.get(), &HealthController::index));
        
        router.get("/users", bind_handler(ctx->usersController.get(), &UsersController::index));
        router.post("/users", bind_handler(ctx->usersController.get(), &UsersController::store));

        // DI example
        // r.get("/users", [repo = std::make_shared<UserRepo>(ctx.pg)]
        //     (Request& req) -> net::awaitable<Outcome> { ... });
    }
}
