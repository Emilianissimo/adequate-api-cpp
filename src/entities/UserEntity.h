#pragma once
#include <optional>
#include <string>
#include <cstdint>

struct UserEntity {
    std::int64_t id;
    std::string  username;
    std::string email;
    std::optional<std::string> picture;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};
