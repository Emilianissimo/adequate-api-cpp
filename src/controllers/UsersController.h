#pragma once
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "services/users/UsersService.h"
#include "core/http/interfaces/HttpInterface.h"
#include "core/openapi/OpenApiRegistrar.h"
#include "core/openapi/types/OpenApiSchemaRegistry.h"

class UsersController {
public:
    static void registerOpenApi()
    {
        OpenApiRegistrar::registerSchema<UserCreateSerializer>();
        OpenApiRegistrar::registerSchema<UserCreateResponseSerializer>();
        OpenApiRegistrar::registerSchema<UserSerializer>();

        // Array registry
        OpenApiSchemaRegistry::instance().registerSchema(
            "UserResponseArray",
            Json{
                {"type","array"},
                {"items", Json{{"$ref", "#/components/schemas/UserResponse"}}}
            }
        );

        // multipart update - manually:
        OpenApiSchemaRegistry::instance().registerSchema("UserUpdateMultipartRequest", {
            {"type","object"},
            {"properties",{
                {"username", {{"type","string"},{"minLength",1}}},
                {"email", {{"type","string"},{"format","email"}}},
                {"password", {{"type","string"},{"minLength",6}}},
                {"picture", {{"type","string"},{"format","binary"}}}
            }},
            {"additionalProperties", false}
        });
    }

    explicit UsersController(UsersService& service) : service_(service) {}
    [[nodiscard]] net::awaitable<Outcome> index(const Request& request) const;
    net::awaitable<Outcome> store(const Request& request) const;
    net::awaitable<Outcome> update(const Request& request) const;
    net::awaitable<Outcome> remove(const Request& request) const;

private:
    UsersService& service_;
};
