#pragma once
#include <vector>
#include "serializers/users/UserSerializer.h"
#include "serializers/users/UserCreateSerializer.h"
#include "serializers/users/UserUpdateSerializer.h"
#include "filters/users/UserListFilter.h"
#include "core/interfaces/HttpInterface.h"
#include "repositories/users/UsersRepository.h"

class UsersService {
public:
    explicit UsersService(UsersRepository& repo) : repo_(repo) {}
    net::awaitable<std::vector<UserSerializer>> list(UserListFilter& filters) const;
    net::awaitable<UserCreateResponseSerializer> create(UserCreateSerializer& data) const;
    net::awaitable<bool> exists(UserFilter& filters) const;

    static net::awaitable<void> update(UserUpdateSerializer& data);
private:
    UsersRepository& repo_;
};
