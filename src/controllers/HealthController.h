#pragma once
#include "core/http/interfaces/HttpInterface.h"
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "core/openapi/types/OpenApiSchemaRegistry.h"

class HealthController {
public:
    static void registerOpenApi() {

        OpenApiSchemaRegistry::instance().registerSchema(
            "HealthResponse",
            {
                {"type","object"},
                {"required", {"status"}},
                {"properties", {
                    {"status", {{"type","string"}}}
                }},
                {"additionalProperties", false}
            }
        );
    }

    /// Note: Controller methods should not be static
    net::awaitable<Outcome> index(const Request& request);
};
