#pragma once
#include "serializers/users/UserSerializer.h"
#include "core/repositories/BaseRepository.h"
#include "filters/users/UserListFilter.h"

class UsersRepository : BaseRepository {
public:
    using BaseRepository::BaseRepository;
    net::awaitable<std::vector<UserEntity>> get_list(UserListFilter& filters);
    net::awaitable<void> create(UserEntity& entity);
};
