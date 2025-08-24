#include "core/serializers/BaseSerializer.h"
#include <string>
#include <optional>

struct UserSerializer : BaseSerializer<UserSerializer> {
    int id;
    std::string name;
    std::optional<std::string> email;

    // Macros should live only in header file
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserSerializer, id, name, email)
};
