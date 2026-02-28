#include "routes/Routes.h"
#include "controllers/HealthController.h"
#include "controllers/UsersController.h"
#include "core/openapi/controllers/SwaggerController.h"
#include "core/openapi/services/SwaggerService.h"
#include "middlewares/AuthenticationMiddleware.h"

namespace app{
    void define_routes(Router& router, const std::shared_ptr<AppContext>& ctx){
        /// Middlewares
        /// Global
        // router.use(std::make_shared<RequestIdMiddleware>());
        // router.use(std::make_shared<LoggingMiddleware>());
        /// Scoped
        router.get(
            "/health",
            bind_handler(ctx->healthController.get(), &HealthController::index),
            OpenApiMeta{
            "Health check endpoint",
            {
                {static_cast<int>(http::status::ok), "Service healthy", "HealthResponse"}
                }
            }
        );

        /// Authentication
        router.post("/register", bind_handler(
            ctx->authenticationController.get(), &AuthenticationController::registration
        ));
        router.post("/login", bind_handler(
            ctx->authenticationController.get(), &AuthenticationController::login
        ));

        router.use("/users", ctx->authenticationMiddleware);
        /// Users CRUD
        router.get("/users", bind_handler(ctx->usersController.get(), &UsersController::index));
        router.post("/users", bind_handler(ctx->usersController.get(), &UsersController::store));
        router.patch(
            "/users/{id}",
            bind_handler(ctx->usersController.get(), &UsersController::update),
            {"multipart/form-data"}
        );
        router.delete_(
            "/users/{id}",
            bind_handler(ctx->usersController.get(), &UsersController::remove)
        );

        // Swagger docs
        router.get(
            "/swagger",
            bind_handler(ctx->swaggerController.get(), &SwaggerController::index)
        );

        HealthController::registerOpenApi();
        SwaggerService::generate(router, ctx->rootPath);
    };
}
