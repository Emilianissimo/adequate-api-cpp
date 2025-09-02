#include "core/serializers/BaseSerializer.h"
#include "entities/UserEntity.h"
#include "helpers/DatetimeConverter.h"
#include <string>
#include <optional>

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

    // Macros should live only in header file
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserSerializer, id, username, picture, created_at)
};
