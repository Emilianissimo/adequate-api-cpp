#pragma once
#include <optional>
#include <string>
#include <regex>
#include <nlohmann/json.hpp>

#include "core/errors/Errors.h"
#include "entities/UserEntity.h"
#include "core/serializers/BaseSerializer.h"
#include "core/loggers/LoggerSingleton.h"

class UserCreateResponseSerializer final : public BaseSerializer<UserCreateResponseSerializer, UserEntity> {
public:
    UserCreateResponseSerializer() = default;

    std::int64_t id{};
    std::string username;
    std::string email;

    static UserCreateResponseSerializer fromEntity(const UserEntity& entity) {
        LoggerSingleton::get().info("UserCreateResponseSerializer::fromEntity: started");
        UserCreateResponseSerializer serialized;
        serialized.id = entity.id;
        serialized.username = entity.username.value();
        serialized.email = entity.email.value();
        return serialized;
    }

    /// Covering to_json / from_json declarative way
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserCreateResponseSerializer, id, username, email)
};

class UserCreateSerializer final : public BaseSerializer<UserCreateSerializer, UserEntity>{
public:
    UserCreateSerializer() = default;

    std::string username;
    std::string email;
    std::string password;

    static UserCreateSerializer fromEntity(const UserEntity& entity) {
        LoggerSingleton::get().info("UserCreateSerializer::fromEntity: started");
        UserCreateSerializer serializer;
        serializer.username = entity.username.value();
        serializer.email = entity.email.value();
        serializer.password = entity.password.value();
        return serializer;
    }

    UserEntity toEntity() {
        LoggerSingleton::get().info("UserCreateSerializer::toEntity: started");
        UserEntity userEntity;
        userEntity.username = this->username;
        userEntity.email = this->email;
        userEntity.password = this->password;
        return userEntity;
    }

    friend void from_json(const nlohmann::json& j, UserCreateSerializer& s) {
        LoggerSingleton::get().debug("UserCreateSerializer::from_json: called", {
            {"json", j.dump()},
        });

        LoggerSingleton::get().debug(
            std::string("UserCreateSerializer::from_json: checking username: ") +
            std::string(j.contains("username") ? j["username"].get<std::string>() : "null")
        );
        if (!j.contains("username") || !j["username"].is_string() || j["username"].get<std::string>().empty()) {
            throw ValidationError("Username is required");
        }

        LoggerSingleton::get().debug(
            std::string("UserCreateSerializer::from_json: checking email: ") +
            std::string(j.contains("email") ? j["email"].get<std::string>() : "null")
        );
        if (!j.contains("email") || !j["email"].is_string() || j["email"].get<std::string>().empty()) {
            throw ValidationError("Email is required");
        }
        if (!std::regex_match(j["email"].get<std::string>(), std::regex(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)"))) {
            throw ValidationError("Email is invalid");
        }

        LoggerSingleton::get().debug("UserCreateSerializer::from_json: checking password");
        if (!j.contains("password") || !j["password"].is_string() || j["password"].get<std::string>().size() < 6) {
            throw ValidationError("Password must be at least 6 characters");
        }

        s.username = j["username"].get<std::string>();
        s.email    = j["email"].get<std::string>();
        s.password = j["password"].get<std::string>();
    }
};
