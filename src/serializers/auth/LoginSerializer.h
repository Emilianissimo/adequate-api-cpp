#ifndef BEAST_API_LOGINSERIALIZER_H
#define BEAST_API_LOGINSERIALIZER_H
#include <regex>
#include "core/serializers/BaseSerializer.h"
#include "entities/UserEntity.h"
#include "core/loggers/LoggerSingleton.h"
#include "core/errors/Errors.h"

class LoginSerializer final : public BaseSerializer<LoginSerializer, UserEntity> {
public:
    LoginSerializer() = default;

    std::string email;
    std::string password;

    static LoginSerializer fromEntity(const UserEntity& entity) {
        LoggerSingleton::get().info("LoginSerializer::fromEntity: started");
        LoginSerializer serializer;
        serializer.email = entity.email.value();
        serializer.password = entity.password.value();
        return serializer;
    }

    [[nodiscard]] UserEntity toEntity() && {
        LoggerSingleton::get().info("LoginSerializer::toEntity (move)");
        UserEntity entity;
        entity.email = std::move(email);
        entity.password = std::move(password);
        return entity;
    }

    [[nodiscard]] UserEntity toEntity() const & {
        LoggerSingleton::get().info("LoginSerializer::toEntity (copy)");
        UserEntity entity;
        entity.email = email;
        entity.password = password;
        return entity;
    }

    friend void from_json(const nlohmann::json& j, LoginSerializer& s) {
        LoggerSingleton::get().info("LoginSerializer::from_json: called");
        LoggerSingleton::get().debug(
            std::string("LoginSerializer::from_json: checking email: ") +
            std::string(j.contains("email") ? j["email"].get<std::string>() : "null")
        );
        if (!j.contains("email") || !j["email"].is_string() || j["email"].get<std::string>().empty()) {
            throw ValidationError("Email is required");
        }
        if (!std::regex_match(j["email"].get<std::string>(), std::regex(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)"))) {
            throw ValidationError("Email is invalid");
        }

        LoggerSingleton::get().debug("LoginSerializer::from_json: checking password");
        if (!j.contains("password") || !j["password"].is_string() || j["password"].get<std::string>().size() < 6) {
            throw ValidationError("Password must be at least 6 characters");
        }

        s.email = j["email"].get<std::string>();
        s.password = j["password"].get<std::string>();
    }
};


#endif //BEAST_API_LOGINSERIALIZER_H
