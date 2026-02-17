#include "services/users/UsersService.h"
#include "core/loggers/LoggerSingleton.h"
#include <algorithm>

#include "core/helpers/Offload.h"

net::awaitable<std::vector<UserSerializer>> UsersService::list(UserListFilter& filters, std::string host) const {
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
    std::vector<UserEntity> users = co_await repo_.getList(filters);
    std::vector<UserSerializer> serializedUsers;
    serializedUsers.reserve(users.size());

    LoggerSingleton::get().debug("Filling response serializer with data");
    // We can retrieve various variants of data, modified too

    for (UserEntity& user : users) {
        if (user.picture.has_value())
        {
            user.picture = fs_.buildAbsolutePath(
                host,
                "users",
                std::to_string(user.id),
                user.picture.value()
            );
        }
        UserSerializer serializedUser = UserSerializer::fromEntity(user);
        serializedUsers.emplace_back(std::move(serializedUser));
    }
    co_return serializedUsers;
}

net::awaitable<UserCreateResponseSerializer> UsersService::create(UserCreateSerializer data) const {
    LoggerSingleton::get().info("UsersService::create: called", {
        {"email", data.email},
        {"password", data.password},
        {"username", data.username}
    });
    UserEntity user = data.toEntity();
    if (user.password.has_value())
    {
        auto pwdPtr = std::make_shared<std::string>(user.password.value());
        auto hasher = passwordHasher_; // copy shared_ptr

        auto hash = co_await async_offload(
        blockingPool_.get_executor(),
            [pwdPtr, hasher]() {
                return hasher->hash(*pwdPtr);
            }
        );
        user.password = std::move(hash);
    }

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

net::awaitable<void> UsersService::update(UserUpdateSerializer data, IncomingFile picture) const
{
    LoggerSingleton::get().info("UsersService::update: called", {
        {"id", data.id},
        {"email", data.email.value_or("null")},
        {"username", data.username.value_or("null")},
        {"picture", data.picture.value_or(("null"))}
    });
    UserEntity user = data.toEntity();

    LoggerSingleton::get().debug("UsersService::update: Updating user. Optional passed params are: ", {
        {"id", user.id},
        {"email", user.email.value_or("null")},
        {"username", user.username.value_or("null")},
        {"picture", data.picture.value_or(("null"))}
    });

    if (user.password.has_value())
    {
        auto pwdPtr = std::make_shared<std::string>(user.password.value());
        auto hasher = passwordHasher_; // copy shared_ptr
        auto hash = co_await async_offload(
            blockingPool_.get_executor(),
            [pwdPtr, hasher]() {
                return hasher->hash(*pwdPtr);
            }
        );
        user.password = std::move(hash);
    }

    try
    {
        co_await repo_.update(user);
        if (user.picture.has_value() && !picture.bytes.empty())
        {
            // Use the current coroutine's executor for non-blocking I/O
            auto* fileSystemHandler = &fs_;
            auto picShared = std::make_shared<IncomingFile>(picture);
            auto userIdShared = std::make_shared<std::string>(std::to_string(user.id));
            co_await async_offload(
                blockingPool_.get_executor(),
                [fileSystemHandler, picShared, userIdShared]()
                {
                    return fileSystemHandler->store(
                        "users",
                        *userIdShared,
                        *picShared
                    );
                }
            );
        }
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
}

net::awaitable<bool> UsersService::exists(UserFilter& filters) const {
    LoggerSingleton::get().info(
        "UsersService::exists: started", {
            {"email", filters.email.value_or("null")},
        }
    );
    co_return co_await repo_.exists(filters);
}
