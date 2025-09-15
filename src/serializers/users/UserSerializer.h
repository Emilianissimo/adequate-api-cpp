#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "entities/UserEntity.h"
#include "core/serializers/BaseSerializer.h"
#include "helpers/DatetimeConverter.h"

class UserSerializer final : public BaseSerializer<UserSerializer, UserEntity> {
public:
    UserSerializer() = default;

    std::int64_t id;
    std::string username;
    std::string email;
    std::optional<std::string> picture;
    std::string created_at;
    std::string updated_at;

    static UserSerializer fromEntity(const UserEntity& entity) {
        UserSerializer userSerializer;
        userSerializer.id = entity.id;
        userSerializer.username = entity.username.value_or("");
        userSerializer.email = entity.email.value_or("");
        userSerializer.picture = entity.picture;
        userSerializer.created_at = to_iso_string(entity.created_at);
        userSerializer.updated_at = to_iso_string(entity.updated_at);
        return userSerializer;
    }

    UserEntity toEntity() {
        UserEntity userEntity;
        userEntity.id = this->id;
        userEntity.username = this->username;
        userEntity.picture = this->picture;
        userEntity.email = this->email;
        userEntity.created_at = parse_pg_timestamp(this->created_at);
        userEntity.updated_at = parse_pg_timestamp(this->updated_at);
        return userEntity;
    }

    /// For current optional picture field we must provide custom to_json
    friend void to_json(nlohmann::json& j, const UserSerializer& s) {
        j = nlohmann::json{
            {"id", s.id},
            {"username", s.username},
            {"email", s.email},
            {"created_at", s.created_at},
            {"updated_at", s.updated_at}
        };
        // optional -> null or string
        j["picture"] = s.picture.has_value()
            ? nlohmann::json(*s.picture)
            : nlohmann::json(nullptr);
    }

    /// In other cases macros with be enough. Macros should live only in header file
    // NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserSerializer, id, username, picture, created_at)
};
