#pragma once
#include <nlohmann/json.hpp>

template <class Derived, class EntityT>
struct BaseSerializer {
    static Derived fromEntity(const EntityT& entity) {
        return Derived::fromEntity(entity);
    }

    EntityT toEntity() const & {
        return static_cast<const Derived*>(this)->toEntity();
    }

    EntityT toEntity() && {
        return static_cast<Derived*>(this)->toEntity();
    }

    nlohmann::json to_json() const {
        return nlohmann::json(static_cast<const Derived&>(*this));
    }

     Derived from_json(const nlohmann::json& data) {
        return data.get<Derived>();
    }
};
