#pragma once
#include <boost/asio/awaitable.hpp>
#include <optional>
#include <vector>
#include "serializers/users/UserSerializer.h"
#include "serializers/users/UserCreateSerializer.h"
#include "filters/users/UserListFilter.h"

namespace net = boost::asio;

class UsersServiceInterface {
public:
    virtual ~UsersServiceInterface() = default;
    virtual net::awaitable<std::vector<UserSerializer>> list(UserListFilter& filters) = 0;
    virtual net::awaitable<UserCreateResponseSerializer> create(UserCreateSerializer& data) = 0;
};
