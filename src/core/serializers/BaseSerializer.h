#pragma once
#include "SerializerInterface.h"
#include <nlohmann/json.hpp>

template <class Derived>
struct BaseSerializer : SerializerInterface {
    nlohmann::json to_json() const override {
        return nlohmann::json(static_cast<const Derived&>(*this));
    }

    static Derived from_json(const nlohmann::json& data) override {
        return data.get<Derived>();
    }
};
