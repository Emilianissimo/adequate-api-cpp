//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_OPENAPISCHEMAREGISTRY_H
#define BEAST_API_OPENAPISCHEMAREGISTRY_H

#include <unordered_map>
#include <string>

#include "OpenApiTypes.h"

class OpenApiSchemaRegistry
{
public:
    static OpenApiSchemaRegistry& instance()
    {
        static OpenApiSchemaRegistry instance_;
        return instance_;
    }

    void registerSchema(const std::string& name, const Json& schema)
    {
        schemas_[name] = schema;
    }

    const std::unordered_map<std::string, Json>& schemas() const
    {
        return schemas_;
    }

private:
    std::unordered_map<std::string, Json> schemas_;
};

#endif //BEAST_API_OPENAPISCHEMAREGISTRY_H
