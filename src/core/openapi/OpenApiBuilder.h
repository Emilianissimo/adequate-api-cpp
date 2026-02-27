//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_OPENAPIBUILDER_H
#define BEAST_API_OPENAPIBUILDER_H

#include "types/OpenApiSchemaRegistry.h"

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

                Json responses;

                for (const auto& [code, description, schemaName] : meta.responses)
                {
                    responses[std::to_string(code)] = {
                        {"description", description},
                        {"content", {
                                {"application/json", {
                                    {"schema", {
                                        {"$ref", "#/components/schemas/" + schemaName}
                                    }}
                                }}
                        }}
                    };
                }

                documentation["paths"][route.original][std::string(http::to_string(method))] = Json{
                {"summary", meta.summary},
                {"responses", responses}
                };
            }
        }

        // Schemas
        Json schemas;
        for (const auto& [name, schema] :
             OpenApiSchemaRegistry::instance().schemas())
        {
            schemas[name] = schema;
        }

        documentation["components"]["schemas"] = schemas;

        return documentation;
    }
};

#endif //BEAST_API_OPENAPIBUILDER_H
