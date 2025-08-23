#include "BaseSerializer.h"
#include <string>
#include <optional>

struct UserSerializer : BaseSerializer<UserSerializer> {
    int id;
    std::string name;
    std::optional<std::string> email;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserSerializer, id, name, email)
};
