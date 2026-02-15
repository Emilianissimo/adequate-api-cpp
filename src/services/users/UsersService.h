#pragma once
#include <vector>
#include "serializers/users/UserSerializer.h"
#include "serializers/users/UserCreateSerializer.h"
#include "serializers/users/UserUpdateSerializer.h"
#include "filters/users/UserListFilter.h"
#include "../../core/http/interfaces/HttpInterface.h"
#include "core/file_system/FileSystemService.h"
#include "repositories/users/UsersRepository.h"
#include <boost/asio/thread_pool.hpp>

class UsersService {
public:
    explicit UsersService(
        UsersRepository& repo,
        FileSystemService& fs,
        boost::asio::thread_pool& blockingPool) : repo_(repo), fs_(fs), blockingPool_(blockingPool) {}
    net::awaitable<std::vector<UserSerializer>> list(UserListFilter& filters) const;
    net::awaitable<UserCreateResponseSerializer> create(UserCreateSerializer& data) const;
    net::awaitable<bool> exists(UserFilter& filters) const;

    net::awaitable<void> update(UserUpdateSerializer& data, IncomingFile& picture) const;
private:
    UsersRepository& repo_;
    FileSystemService& fs_;
    boost::asio::thread_pool& blockingPool_;
};
