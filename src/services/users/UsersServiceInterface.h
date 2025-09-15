#pragma once
#include <boost/asio/awaitable.hpp>
#include <vector>
#include "serializers/users/UserSerializer.h"
#include "serializers/users/UserCreateSerializer.h"
#include "serializers/users/UserUpdateSerializer.h"
#include "filters/users/UserListFilter.h"
#include "core/interfaces/HttpInterface.h"

class UsersServiceInterface {
public:
    virtual ~UsersServiceInterface() = default;
    virtual net::awaitable<std::vector<UserSerializer>> list(UserListFilter& filters) = 0;
    virtual net::awaitable<UserCreateResponseSerializer> create(UserCreateSerializer& data) = 0;
    virtual net::awaitable<void> update(UserUpdateSerializer& data) = 0;
};
