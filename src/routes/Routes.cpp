#include "routes/Routes.h"
#include "controllers/HealthController.h"
#include "controllers/UsersController.h"
#include "core/openapi/controllers/SwaggerController.h"
#include "core/openapi/services/SwaggerService.h"
#include "core/openapi/types/OpenApiResponses.h"
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
            .summary="Health check endpoint",
            .responses=OpenApiResponses{}
                .ok("HealthResponse")
                .withAlways()
                .build()
            }
        );

        /// Authentication
        router.post("/register", bind_handler(
            ctx->authenticationController.get(), &AuthenticationController::registration
            ), {},
            OpenApiMeta{
                .summary = "Register",
                .responses=OpenApiResponses{}
                    .created()
                    .withAlways()
                    .build(),
                .requestBody = OpenApiRequestBody{
                    .contentType="application/json",
                    .schemaRef="RegisterRequest",
                    .required=true
                }
            }
        );
        router.post("/login", bind_handler(
            ctx->authenticationController.get(), &AuthenticationController::login
        ), {},
            OpenApiMeta{
                .summary = "Login/Obtain tokens",
                .responses=OpenApiResponses{}
                    .ok("LoginResponse")
                    .withAlways()
                    .build(),
                .requestBody = OpenApiRequestBody{
                    .contentType="application/json",
                    .schemaRef="LoginRequest",
                    .required=true
                }
            }
        );

        router.use("/users", ctx->authenticationMiddleware);
        /// Users CRUD
        router.get(
            "/users",
            bind_handler(ctx->usersController.get(), &UsersController::index),
            OpenApiMeta{
                .summary = "List users",
                .responses = OpenApiResponses{}
                    .okArray("UserResponse")
                    .withAuth()
                    .withAlways()
                    .build(),
                .parameters = OpenApiFilterSpec<UserListFilter>::parameters(),
                .authRequired = true
            }
        );
        router.post(
            "/users",
            bind_handler(ctx->usersController.get(), &UsersController::store), {},
            OpenApiMeta{
                .summary = "Create user",
                .responses = OpenApiResponses{}
                    .created("UserCreateResponse")
                    .withAuth()
                    .withBody()
                    .withAlways()
                    .build(),
                .requestBody = OpenApiRequestBody{
                    .contentType="application/json",
                    .schemaRef="UserCreateRequest",
                    .required=true
                },
                .authRequired = true
            }
        );

        /// Parameter declaration
        auto params = nlohmann::json::array();
        params.push_back(OpenApiParamBuilder::path("id", {{"type","integer"},{"format","int64"}}, "User id"));
        router.patch(
            "/users/{id}",
            bind_handler(ctx->usersController.get(), &UsersController::update),
            {"multipart/form-data"},
            OpenApiMeta{
                .summary="Update user",
                .responses=OpenApiResponses{}
                    .noContent()
                    .withAuth()
                    .withAlways()
                    .notFound()
                    .withBody()
                    .build(),
                .parameters=params,
                .requestBody=OpenApiRequestBody{
                    .contentType="multipart/form-data",
                    .schemaRef="UserUpdateMultipartRequest",
                    .required=true
                },
                .authRequired=true
            }
        );

        router.delete_(
            "/users/{id}",
            bind_handler(ctx->usersController.get(), &UsersController::remove), {},
            OpenApiMeta{
                .summary="Delete user",
                .responses=OpenApiResponses{}
                    .noContent()
                    .withAuth()
                    .withAlways()
                    .notFound()
                    .build(),
                .parameters=params,
                .authRequired=true
            }
        );

        // Swagger docs
        router.get(
            "/swagger",
            bind_handler(ctx->swaggerController.get(), &SwaggerController::index)
        );

        /// OpenApi registration for controllers
        HealthController::registerOpenApi();
        UsersController::registerOpenApi();
        AuthenticationController::registerOpenApi();

        /// Basic error response
        OpenApiSchemaRegistry::instance().registerSchema("ErrorResponse",{
            {"type", "object"},
            {"required", nlohmann::json::array({"error"})},
            {"properties", {
                {"error", {{"type", "string"}}}
            }},
            {"additionalProperties", false}
        });

        /// Swagger generation
        SwaggerService::generate(router, ctx->rootPath);
    };
}
