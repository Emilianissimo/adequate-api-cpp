#pragma once
#ifndef BEAST_API_REGISTERSERIALIZER_H
#define BEAST_API_REGISTERSERIALIZER_H
#include <regex>
#include "core/serializers/BaseSerializer.h"
#include "entities/UserEntity.h"
#include "core/loggers/LoggerSingleton.h"
#include "core/errors/Errors.h"

class RegisterSerializer final : public BaseSerializer<RegisterSerializer, UserEntity> {
public:
    RegisterSerializer() = default;

    std::string username;
    std::string email;
    std::string password;


    static RegisterSerializer fromEntity(const UserEntity& entity) {
        LoggerSingleton::get().info("RegisterSerializer::fromEntity: started");
        RegisterSerializer serializer;
        serializer.username = entity.username.value();
        serializer.email = entity.email.value();
        serializer.password = entity.password.value();
        return serializer;
    }

    UserEntity toEntity() const {
        LoggerSingleton::get().info("RegisterSerializer::toEntity");
        UserEntity entity;
        entity.username = username;
        entity.email = email;
        entity.password = password;
        return entity;
    }

    friend void from_json(const nlohmann::json& j, RegisterSerializer& s) {
        LoggerSingleton::get().info("RegisterSerializer::from_json: called");

        LoggerSingleton::get().debug(
            std::string("RegisterSerializer::from_json: checking username: ") +
            std::string(j.contains("username") ? j["username"].get<std::string>() : "null")
        );
        if (!j.contains("username") || !j["username"].is_string() || j["username"].get<std::string>().empty()) {
            throw ValidationError("Username is required");
        }
        LoggerSingleton::get().debug(
            std::string("RegisterSerializer::from_json: checking email: ") +
            std::string(j.contains("email") ? j["email"].get<std::string>() : "null")
        );
        if (!j.contains("email") || !j["email"].is_string() || j["email"].get<std::string>().empty()) {
            throw ValidationError("Email is required");
        }
        if (!std::regex_match(j["email"].get<std::string>(), std::regex(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)"))) {
            throw ValidationError("Email is invalid");
        }

        LoggerSingleton::get().debug("RegisterSerializer::from_json: checking password");
        if (!j.contains("password") || !j["password"].is_string() || j["password"].get<std::string>().size() < 6) {
            throw ValidationError("Password must be at least 6 characters");
        }

        s.email = j["email"].get<std::string>();
        s.password = j["password"].get<std::string>();
    }
};


#endif //BEAST_API_REGISTERSERIALIZER_H
