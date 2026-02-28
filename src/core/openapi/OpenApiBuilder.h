//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_OPENAPIBUILDER_H
#define BEAST_API_OPENAPIBUILDER_H

#include "types/OpenApiSchemaRegistry.h"
#include "core/http/interfaces/HttpInterface.h"
#include <cctype>
#include <algorithm>

inline std::vector<OpenApiResponseMeta> withCommonErrors(
    std::vector<OpenApiResponseMeta> ok,
    const bool authRequired,
    const bool hasBody
) {
    auto add = [&](const int status, std::string desc, std::optional<std::string> schema = "ErrorResponse") {
        ok.push_back({status, std::move(desc), std::move(schema)});
    };

    add(static_cast<int>(http::status::internal_server_error), "Internal server error");

    if (authRequired) {
        add(static_cast<int>(http::status::unauthorized), "Unauthorized");
        add(static_cast<int>(http::status::forbidden), "Forbidden");
    }
    if (hasBody) {
        add(static_cast<int>(http::status::bad_request), "Bad request / validation error");
        add(static_cast<int>(http::status::payload_too_large), "Payload too large");
        add(static_cast<int>(http::status::unsupported_media_type), "Unsupported media type");
    }
    return ok;
}

class OpenApiBuilder
{
public:
    static Json build(
        const Router& router,
        const std::string& title = "CPP API",
        const std::string& version = "1.0.0"
    )
    {
        Json documentation;

        documentation["openapi"] = "3.0.3";
        documentation["info"] = {
            {"title", title},
            {"version", version}
        };

        // Paths
        for (const auto& route : router.routes())
        {
            for (const auto& method : route.methods | std::views::keys)
            {
                if (!route.openapiMeta.contains(method))
                    continue;

                const auto& meta = route.openapiMeta.at(method);

                Json responses = Json::object();

                for (const auto& [code, description, schemaName] : meta.responses)
                {
                    if (schemaName.has_value())
                    {
                        responses[std::to_string(code)] = {
                            {"description", description},
                            {"content", {
                                    {"application/json", {
                                        {"schema", {
                                            {"$ref", "#/components/schemas/" + schemaName.value()}
                                        }}
                                    }}
                            }}
                        };
                    }

                }

                // ---- operation object
                Json operation = {
                    {"summary", meta.summary},
                    {"responses", responses}
                };

                // ---- parameters (query/path)
                if (!meta.parameters.is_null() && meta.parameters.is_array() && !meta.parameters.empty())
                {
                    operation["parameters"] = meta.parameters;
                }

                // ---- requestBody
                if (meta.requestBody.has_value())
                {
                    const auto& rb = *meta.requestBody;

                    operation["requestBody"] = {
                        {"required", rb.required},
                        {"content", {
                                {rb.contentType, {
                                    {"schema", {
                                        {"$ref", std::string("#/components/schemas/") + rb.schemaRef}
                                    }}
                                }}
                        }}
                    };
                }

                // ---- security
                if (meta.authRequired)
                {
                    // Assumes you define bearerAuth in components.securitySchemes somewhere
                    operation["security"] = Json::array({ Json{{"bearerAuth", Json::array()}} });
                }

                // method string -> lowercase
                auto methodStr = std::string(http::to_string(method));
                std::ranges::transform(
                    methodStr, methodStr.begin(),
                    [](const unsigned char c){ return static_cast<char>(std::tolower(c)); }
                );

                documentation["paths"][route.original][methodStr] = std::move(operation);
            }
        }

        // Schemas
        Json schemas = Json::object();
        for (const auto& [name, schema] :
             OpenApiSchemaRegistry::instance().schemas())
        {
            schemas[name] = schema;
        }

        documentation["components"]["schemas"] = schemas;

        // Security schemes (optional but recommended if you use authRequired)
        // If you DON'T add this, Swagger UI will still show authRequired,
        // but "Authorize" button won't work properly.
        documentation["components"]["securitySchemes"]["bearerAuth"] = {
            {"type", "http"},
            {"scheme", "bearer"},
            {"bearerFormat", "JWT"}
        };

        return documentation;
    }
};

#endif //BEAST_API_OPENAPIBUILDER_H
