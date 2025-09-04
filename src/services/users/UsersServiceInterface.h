#pragma once
#include <boost/asio/awaitable.hpp>
#include <optional>
#include <vector>
#include "serializers/users/UserSerializer.h"
#include "serializers/users/UserCreateSerializer.h"

namespace net = boost::asio;

class UsersServiceInterface {
public:
    virtual ~UsersServiceInterface() = default;
    virtual net::awaitable<std::vector<UserSerializer>> list(std::size_t limit, std::size_t offset) = 0;
    virtual net::awaitable<UserCreateResponseSerializer> create(UserCreateSerializer& data) = 0;
};
