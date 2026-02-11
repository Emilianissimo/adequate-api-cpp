#pragma once
#include <stdexcept>
#include <string>

enum class DbErrorCode {
    UniqueViolation,
    ForeignKeyViolation,
    NotNullViolation,
    Unknown
};

inline DbErrorCode map_sqlstate(const std::string& state) {
    if (state == "23505") return DbErrorCode::UniqueViolation;
    if (state == "23503") return DbErrorCode::ForeignKeyViolation;
    if (state == "23502") return DbErrorCode::NotNullViolation;
    return DbErrorCode::Unknown;
}

class DbError final : public std::runtime_error {
public:
    explicit DbError(const DbErrorCode code, const std::string& msg)
        : std::runtime_error(msg), code_(code) {}

    [[nodiscard]] DbErrorCode code() const noexcept { return code_; }

private:
    DbErrorCode code_;
};

class MultipartError final : public std::runtime_error {
public:
    explicit MultipartError(const std::string& message)
        : std::runtime_error(message) {}
};

class ValidationError final : public std::runtime_error {
public:
    explicit ValidationError(const std::string& message)
        : std::runtime_error(message) {}
};

