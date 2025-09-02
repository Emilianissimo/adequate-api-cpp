#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "entities/UserEntity.h"
#include "core/serializers/BaseSerializer.h"
#include "helpers/DatetimeConverter.h"

class UserSerializer : public BaseSerializer<UserSerializer> {
public:
    UserSerializer() = default;

    int id;
    std::string username;
    std::optional<std::string> picture;
    std::string email;
    std::string created_at;

    static UserSerializer fromEntity(const UserEntity& entity) {
        UserSerializer userSerializer;
        userSerializer.id = entity.id;
        userSerializer.username = entity.username;
        userSerializer.picture = entity.picture;
        userSerializer.email = entity.email;
        userSerializer.created_at = to_iso_string(entity.created_at);
        return userSerializer;
    }

    // for current optinal picture field we must provide custom to_json
    friend void to_json(nlohmann::json& j, const UserSerializer& s) {
        j = nlohmann::json{
            {"id", s.id},
            {"username", s.username},
            {"email", s.email},
            {"created_at", s.created_at}
        };
        // optional -> null or string
        j["picture"] = s.picture.has_value()
            ? nlohmann::json(*s.picture)
            : nlohmann::json(nullptr);
    }
    // In other cases macros with be enough
    // Macros should live only in header file
    // NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserSerializer, id, username, picture, created_at)
};
