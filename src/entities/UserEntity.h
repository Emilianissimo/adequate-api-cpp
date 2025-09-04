#pragma once
#include <optional>
#include <string>
#include <cstdint>
#include "core/interfaces/EntityInterface.h"

struct UserEntity : public EntityInterface {
    std::int64_t id;
    std::string  username;
    std::string email;
    std::optional<std::string> picture;
    std::string password;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};
