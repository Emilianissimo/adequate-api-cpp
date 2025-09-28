#pragma once
#include "serializers/users/UserSerializer.h"
#include "core/repositories/BaseRepository.h"
#include "filters/users/UserListFilter.h"

class UsersRepository : BaseRepository {
public:
    using BaseRepository::BaseRepository;
    net::awaitable<std::vector<UserEntity>> getList(UserListFilter& filters) const;
    net::awaitable<UserEntity> getOne(UserListFilter& filters) const;
    net::awaitable<void> create(UserEntity& entity) const;
    net::awaitable<bool> exists(UserListFilter& filters) const;

    static net::awaitable<void> update(UserEntity& entity);
};
