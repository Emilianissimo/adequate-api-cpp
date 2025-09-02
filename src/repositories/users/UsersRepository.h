#pragma once
#include "serializers/users/UserSerializer.h"
#include "repositories/BaseRepository.h"

class UsersRepository : BaseRepository {
public:
    net::awaitable<std::vector<UserEntity>> get_list(std::size_t limit, std::size_t offset);
};
