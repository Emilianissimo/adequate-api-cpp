#pragma once
#include "core/interfaces/SerializerInterface.h"
#include <nlohmann/json.hpp>

template <class Derived>
struct BaseSerializer : SerializerInterface {
    nlohmann::json to_json() const {
        return nlohmann::json(static_cast<const Derived&>(*this));
    }

    static Derived from_json(const nlohmann::json& data) {
        return data.get<Derived>();
    }
};
