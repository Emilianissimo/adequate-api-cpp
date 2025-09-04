#pragma once
#include <optional>
#include <string>
#include <regex>
#include <nlohmann/json.hpp>

#include "core/errors/Errors.h"
#include "entities/UserEntity.h"
#include "core/serializers/BaseSerializer.h"
#include "helpers/DatetimeConverter.h"
#include "core/loggers/LoggerSingleton.h"

class UserCreateResponseSerializer final : public BaseSerializer<UserCreateResponseSerializer, UserEntity> {
public:
    UserCreateResponseSerializer() = default;

    std::int64_t id;
    std::string username;
    std::optional<std::string> picture;
    std::string email;
    std::string password;

    static UserCreateResponseSerializer fromEntity(const UserEntity& entity) {
        UserCreateResponseSerializer serialized;
        serialized.id = entity.id;
        serialized.username = entity.username;
        serialized.email = entity.email;
        serialized.picture = entity.picture;
        return serialized;
    }

    friend void to_json(nlohmann::json& j, const UserCreateResponseSerializer& s) {
        j = nlohmann::json{
            {"id", s.id},
            {"username", s.username},
            {"email", s.email},
        };
        // optional -> null or string
        j["picture"] = s.picture.has_value()
            ? nlohmann::json(*s.picture)
            : nlohmann::json(nullptr);
    }
};

class UserCreateSerializer final : public BaseSerializer<UserCreateSerializer, UserEntity>{
public:
    UserCreateSerializer() = default;

    std::string username;
    std::optional<std::string> picture;
    std::string email;
    std::string password;

    static UserCreateSerializer fromEntity(const UserEntity& entity) {
        UserCreateSerializer serializer;
        serializer.username = entity.username;
        serializer.picture = entity.picture;
        serializer.email = entity.email;
        serializer.password = entity.password;
        return serializer;
    }

    UserEntity toEntity() {
        UserEntity userEntity;
        userEntity.username = this->username;
        userEntity.picture = this->picture;
        userEntity.email = this->email;
        userEntity.password = this->password;
        return userEntity;
    }

    friend void from_json(const nlohmann::json& j, UserCreateSerializer& s) {
        std::ostringstream ss;
        //TODO: Clean password value from json to log
        ss << "UserCreateSerializer::from_json: called, " << "json=" << j;
        LoggerSingleton::get().info(ss.str());
        ss.clear();

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

        LoggerSingleton::get().debug("UserCreateSerializer::from_json: checking picture");
        if (j.contains("picture") && j["picture"].is_string()) {
            auto pic = j["picture"].get<std::string>();
            if (!pic.empty()) {
                s.picture = pic;
            }
        }
    }
};
