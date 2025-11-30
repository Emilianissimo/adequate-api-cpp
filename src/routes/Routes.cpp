#include "routes/Routes.h"
#include "controllers/HealthController.h"
#include "controllers/UsersController.h"
#include "middlewares/AuthenticationMiddleware.h"

namespace app{
    void define_routes(Router& router, const std::shared_ptr<AppContext>& ctx){
        /// Middlewares
        /// Global
        // router.use(std::make_shared<RequestIdMiddleware>());
        // router.use(std::make_shared<LoggingMiddleware>());
        /// Scoped
        router.get("/health", bind_handler(ctx->healthController.get(), &HealthController::index));

        router.use("/users", ctx->authenticationMiddleware);
        router.get("/users", bind_handler(ctx->usersController.get(), &UsersController::index));
        router.post("/users", bind_handler(ctx->usersController.get(), &UsersController::store));
        router.patch("/users/{id}", bind_handler(ctx->usersController.get(), &UsersController::update));
    }
}
