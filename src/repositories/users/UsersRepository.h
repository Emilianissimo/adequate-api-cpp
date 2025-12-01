#pragma once
#include "serializers/users/UserSerializer.h"
#include "core/repositories/BaseRepository.h"
#include "filters/users/UserFilter.h"
#include "filters/users/UserListFilter.h"

class UsersRepository : BaseRepository {
public:
    using BaseRepository::BaseRepository;
    net::awaitable<std::vector<UserEntity>> getList(UserListFilter& filters) const;
    [[nodiscard]] net::awaitable<UserEntity> getOne(const UserFilter& filters) const;
    net::awaitable<void> create(UserEntity& entity) const;
    [[nodiscard]] net::awaitable<bool> exists(const UserFilter& filters) const;

    static net::awaitable<void> update(UserEntity& entity);
};
