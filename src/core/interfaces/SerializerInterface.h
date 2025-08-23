#pragma once
#include <nlohmann/json.hpp>

struct SerializerInterface {
    virtual ~SerializerInterface() = default;
    virtual nlohmann::json to_json() const = 0;
};
