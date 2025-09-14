#include "services/users/UsersServiceInterface.h"
#include "repositories/users/UsersRepository.h"
#include "core/loggers/LoggerSingleton.h"
#include <algorithm>
#include <iostream>
#include <bcrypt/BCrypt.hpp>

class UsersService final : public UsersServiceInterface {
public:
    explicit UsersService(UsersRepository& repo) : repo_(repo) {}

    net::awaitable<std::vector<UserSerializer>> list(UserListFilter& filters) override {
        LoggerSingleton::get().info("UsersService::list: started");
        filters.limit = std::clamp<std::size_t>(filters.limit.value_or(50), 1, 1000);

        // place for transactions if you need them
        std::vector<UserEntity> users = co_await repo_.get_list(filters);
        std::vector<UserSerializer> serializedUsers;
        serializedUsers.reserve(users.size());

        // We can retrieve various variants of data, modified too
        for (UserEntity& user : users) {
            UserSerializer serializedUser;
            serializedUser.id = user.id;
            serializedUser.username = user.username;
            serializedUser.email = user.email;
            serializedUser.picture = user.picture;
            serializedUser.created_at = to_iso_string(user.created_at);
            serializedUser.updated_at = to_iso_string(user.updated_at);
            serializedUsers.emplace_back(std::move(serializedUser));
        }
        co_return serializedUsers;
    }

    virtual net::awaitable<UserCreateResponseSerializer> create(UserCreateSerializer& data) override {
        std::ostringstream ss;
        ss << "UsersService::create: called, "
        << "data=" << data.email << data.password << data.username;
        LoggerSingleton::get().info(ss.str());
        ss.clear();
        UserEntity user = data.toEntity();
        // hashing password
        user.password = BCrypt::generateHash(user.password);
        
        LoggerSingleton::get().debug(
            "UsersService::create: Creating user by repo"
        );
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

private:
    UsersRepository& repo_;
};
