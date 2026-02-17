#pragma once
#include <vector>
#include "serializers/users/UserSerializer.h"
#include "serializers/users/UserCreateSerializer.h"
#include "serializers/users/UserUpdateSerializer.h"
#include "filters/users/UserListFilter.h"
#include "core/file_system/FileSystemService.h"
#include "repositories/users/UsersRepository.h"
#include <boost/asio/thread_pool.hpp>
#include "core/hashers/SodiumPasswordHasher.h"

class UsersService {
public:
    explicit UsersService(
        UsersRepository& repo,
        FileSystemService& fs,
        net::thread_pool& blockingPool,
        const std::shared_ptr<app::security::SodiumPasswordHasher>& passwordHasher
    ) :
    repo_(repo),
    fs_(fs),
    blockingPool_(blockingPool),
    passwordHasher_(passwordHasher) {}
    net::awaitable<std::vector<UserSerializer>> list(UserListFilter& filters, std::string host) const;
    net::awaitable<UserCreateResponseSerializer> create(UserCreateSerializer data) const;
    net::awaitable<bool> exists(UserFilter& filters) const;

    net::awaitable<void> update(UserUpdateSerializer data, IncomingFile picture) const;
private:
    UsersRepository& repo_;
    FileSystemService& fs_;
    net::thread_pool& blockingPool_;
    std::shared_ptr<app::security::SodiumPasswordHasher> passwordHasher_;
};
