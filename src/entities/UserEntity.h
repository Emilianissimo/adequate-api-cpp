#pragma once
#include <optional>
#include <string>
#include <cstdint>
#include "core/interfaces/EntityInterface.h"

struct UserEntity : public EntityInterface {
    std::int64_t id;
    /// For partial update we may need optional
    std::optional<std::string>  username;
    std::optional<std::string> email;
    std::optional<std::string> picture;
    std::optional<std::string> password;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};
