#include "services/users/UsersServiceInterface.h"
#include "repositories/users/UsersRepository.h"
#include "core/loggers/LoggerSingleton.h"
#include <algorithm>
#include <bcrypt/BCrypt.hpp>

class UsersService final : public UsersServiceInterface {
public:
    explicit UsersService(UsersRepository& repo) : repo_(repo) {}

    net::awaitable<std::vector<UserSerializer>> list(UserListFilter& filters) override {
        LoggerSingleton::get().info(
            "UsersService::list: started", {
                {"id", filters.limit.has_value() ? std::to_string(filters.limit.value()) : "null"},
                {"id__in", filters.id__in.value_or(std::vector<int64_t>{})},
                {"username", filters.username.value_or("null")},
                {"username__in", filters.username__in.value_or(std::vector<std::string>{})},
                {"email", filters.email.value_or("null")},
                {"limit", filters.limit.has_value() ? std::to_string(filters.limit.value()) : "null"},
                {"offset", filters.offset.has_value() ? std::to_string(filters.offset.value()) : "null"}
            }
        );
        filters.limit = std::clamp<std::size_t>(filters.limit.value_or(50), 1, 1000);

        // place for transactions if you need them
        std::vector<UserEntity> users = co_await repo_.get_list(filters);
        std::vector<UserSerializer> serializedUsers;
        serializedUsers.reserve(users.size());

        LoggerSingleton::get().debug("Filling response serializer with data");
        // We can retrieve various variants of data, modified too

        for (UserEntity& user : users) {
            UserSerializer serializedUser = UserSerializer::fromEntity(user);
            serializedUsers.emplace_back(std::move(serializedUser));
        }
        co_return serializedUsers;
    }

    virtual net::awaitable<UserCreateResponseSerializer> create(UserCreateSerializer& data) override {
        LoggerSingleton::get().info("UsersService::create: called", {
            {"email", data.email},
            {"password", data.password},
            {"username", data.username}
        });
        UserEntity user = data.toEntity();
        user.password = BCrypt::generateHash(user.password.value());
        
        LoggerSingleton::get().debug("UsersService::create: Creating user by repo");
        try {
            co_await repo_.create(user);
            co_return UserCreateResponseSerializer::fromEntity(user);
        } catch (const DbError& e) {
            LoggerSingleton::get().warn(
                "UsersService::create: Error of creating user " + std::to_string(static_cast<int>(e.code())) +
                ", msg=" + e.what()
            );
            switch (e.code()) {
                case DbErrorCode::UniqueViolation:
                    throw ValidationError("User already exists");
                case DbErrorCode::NotNullViolation:
                    throw ValidationError("Missing required fields");
                default:
                    throw; // Others are 500
            }
        }
    }

    virtual net::awaitable<void> update(UserUpdateSerializer& data) override
    {
        LoggerSingleton::get().info("UsersService::update: called", {
            {"id", data.id},
            {"email", data.email.value_or("null")},
            {"username", data.username.value_or("null")},
            {"picture", data.picture.has_value() ? std::string("passed") : std::string("null")}
        });
        UserEntity user = data.toEntity();

        LoggerSingleton::get().debug("UsersService::update: Updating user. Optional passed params are: ", {
            {"id", user.id},
            {"email", user.email.value_or("null")},
            {"username", user.username.value_or("null")},
            {"picture", data.picture.has_value() ? std::string("passed") : std::string("null")}
        });

        if (user.password.has_value())
        {
            user.password = BCrypt::generateHash(user.password.value());
        }

        try
        {
            co_await repo_.update(user);
            co_return;
        } catch (const DbError& e)
        {
            LoggerSingleton::get().warn(
                "UsersService::update: Error of updating user " + std::to_string(static_cast<int>(e.code())) +
                ", msg=" + e.what()
            );
            switch (e.code()) {
                case DbErrorCode::UniqueViolation:
                    throw ValidationError("Email already exists");
                case DbErrorCode::NotNullViolation:
                    throw ValidationError("Missing required fields");
                default:
                    throw; // Others are 500
            }
        }
    };

private:
    UsersRepository& repo_;
};
