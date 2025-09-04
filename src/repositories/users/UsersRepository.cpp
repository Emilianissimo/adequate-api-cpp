#include "repositories/users/UsersRepository.h"
#include "helpers/DatetimeConverter.h"

net::awaitable<std::vector<UserEntity>> UsersRepository::get_list(std::size_t limit, std::size_t offset){
    std::vector<std::optional<std::string>> params;
    params.emplace_back(std::to_string(limit));
    params.emplace_back(std::to_string(offset));

    auto result = co_await pool_->query(
        "SELECT id, username, picture, email, created_at, updated_at "
        "FROM users ORDER BY id "
        "LIMIT $1::int OFFSET $2::int;",
        params,
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
    params.emplace_back(entity.picture);
    params.emplace_back(entity.password);
    auto result = co_await pool_->query(
        "INSERT INTO users (username, email, picture, password) "
        "VALUES ($1,$2,$3,$4) RETURNING id;",
        params,
        std::chrono::seconds(5)
    );

    if (!result.rows.empty() && !result.rows[0].columns[0].is_null) {
        entity.id = std::stoll(result.rows[0].columns[0].data);
    }
}
