#include "repositories/users/UsersRepository.h"
#include "helpers/DatetimeConverter.h"
#include "core/db/postgres/builder/SQLBuilder.h"

net::awaitable<std::vector<UserEntity>> UsersRepository::get_list(UserListFilter& filters){
    SQLBuilder qb("SELECT id, username, picture, email, created_at, updated_at FROM users");
    if (filters.id.has_value()) {
        qb.where("id", filters.id.value_or(0));
    }
    if (filters.id__in.has_value()) {
        qb.whereAny("id", "bigint", filters.id__in.value());
    }
    if (filters.username__in.has_value()) {
        qb.whereAny("username", "text", filters.username__in.value());
    }
    if (filters.username.has_value()) {
        qb.where("username", filters.username.value());
    }
    if (filters.email.has_value()) {
        qb.where("email", filters.email.value());
    }

    qb.orderBy("id");

    if (filters.limit.has_value()) {
        qb.limit(filters.limit.value());
    }
    if (filters.offset.has_value()) {
        qb.limit(filters.offset.value());
    }

    auto result = co_await pool_->query(
        qb.str(),
        qb.params(),
        std::chrono::seconds(5)
    );

    std::vector<UserEntity> users;
    users.reserve(result.rows.size());

    for (auto& row : result.rows) {
        UserEntity user;
        user.id = std::stoll(row.columns[0].data);
        user.username = row.columns[1].data;
        user.picture = row.columns[2].is_null
            ? std::nullopt
            : std::make_optional(row.columns[2].data);
        user.email = row.columns[3].data;
        user.created_at = parse_pg_timestamp(row.columns[4].data);
        user.updated_at = parse_pg_timestamp(row.columns[5].data);
        users.push_back(std::move(user));
    }

    co_return users;
}

net::awaitable<void> UsersRepository::create(UserEntity& entity) {
    std::vector<std::optional<std::string>> params;
    // Password should be encrypted in service
    params.emplace_back(entity.username);
    params.emplace_back(entity.email);
    params.emplace_back(entity.password);
    auto result = co_await pool_->query(
        "INSERT INTO users (username, email, password) "
        "VALUES ($1,$2,$3) RETURNING id;",
        params,
        std::chrono::seconds(5)
    );

    if (!result.rows.empty() && !result.rows[0].columns[0].is_null) {
        entity.id = std::stoll(result.rows[0].columns[0].data);
    }
}
