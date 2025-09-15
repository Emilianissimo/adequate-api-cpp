#include "core/errors/Errors.h"
#include "entities/UserEntity.h"
#include "core/serializers/BaseSerializer.h"
#include "core/loggers/LoggerSingleton.h"
#include <regex>

class UserUpdateSerializer final : public BaseSerializer<UserUpdateSerializer, UserEntity> {
public:
    UserUpdateSerializer() = default;

    std::int64_t id;
    std::optional<std::string> username;
    std::optional<std::string> picture;
    std::optional<std::string> email;
    std::optional<std::string> password;

    UserEntity toEntity() {
        LoggerSingleton::get().info("UserUpdateSerializer::toEntity: started");
        UserEntity userEntity;
        userEntity.id = this->id;
        userEntity.username = this->username;
        userEntity.picture = this->picture;
        userEntity.email = this->email;
        userEntity.password = this->password;

        return userEntity;
    }

    friend void from_json(const nlohmann::json& j, UserUpdateSerializer& s) {
        LoggerSingleton::get().debug("UserUpdateSerializer::from_json: called", {
            {"json", j.dump()},
        });

        LoggerSingleton::get().debug(
            std::string("UserUpdateSerializer::from_json: checking username: ") +
            std::string(j.contains("username") ? j["username"].get<std::string>() : "null")
        );
        if (j.contains("username") && (!j["username"].is_string() || j["username"].get<std::string>().empty())) {
            throw ValidationError("Username cannot be empty");
        }

        LoggerSingleton::get().debug(
            std::string("UserUpdateSerializer::from_json: checking email: ") +
            std::string(j.contains("email") ? j["email"].get<std::string>() : "null")
        );
        if (j.contains("email") && (!j["email"].is_string() || j["email"].get<std::string>().empty())) {
            throw ValidationError("Email cannot be empty");
        }

        if (j.contains("email") && !std::regex_match(j["email"].get<std::string>(), std::regex(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)"))) {
            throw ValidationError("Email is invalid");
        }

        LoggerSingleton::get().debug("UserUpdateSerializer::from_json: checking password");
        if (j.contains("password") && (!j["password"].is_string() || j["password"].get<std::string>().size() < 6)) {
            throw ValidationError("Password must be at least 6 characters");
        }

        if (j.contains("username")) s.username = j["username"].get<std::string>();
        if (j.contains("email"))    s.email    = j["email"].get<std::string>();
        if (j.contains("password")) s.password = j["password"].get<std::string>();

        LoggerSingleton::get().debug("UserUpdateSerializer::from_json: checking picture");
        if (j.contains("picture") && j["picture"].is_string()) {
            auto pic = j["picture"].get<std::string>();
            if (!pic.empty()) {
                s.picture = pic;
            }
        }
    }
};
