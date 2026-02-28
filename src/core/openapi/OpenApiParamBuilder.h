//
// Created by user on 28.02.2026.
//

#ifndef BEAST_API_OPENAPIPARAMBUILDER_H
#define BEAST_API_OPENAPIPARAMBUILDER_H

#include <nlohmann/json.hpp>

class OpenApiParamBuilder
{
public:
    static nlohmann::json query(
        std::string name,
        nlohmann::json schema,
        std::string description = "",
        bool required = false
    )
    {
        nlohmann::json p{
        {"name", std::move(name)},
        {"in", "query"},
        {"required", required},
        {"schema", std::move(schema)}
        };
        if (!description.empty()) p["description"] = std::move(description);
        return p;
    }

    static nlohmann::json path(
        std::string name,
        nlohmann::json schema,
        std::string description = ""
    )
    {
        nlohmann::json p{
        {"name", std::move(name)},
        {"in", "path"},
        {"required", true},
        {"schema", std::move(schema)}
        };
        if (!description.empty()) p["description"] = std::move(description);
        return p;
    }
};

#endif //BEAST_API_OPENAPIPARAMBUILDER_H
