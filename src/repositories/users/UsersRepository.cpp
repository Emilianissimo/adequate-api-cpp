#include "repositories/users/UsersRepository.h"
#include "helpers/DatetimeConverter.h"
#include "core/db/postgres/builder/SQLBuilder.h"
#include "core/loggers/LoggerSingleton.h"

net::awaitable<std::vector<UserEntity>> UsersRepository::getList(UserListFilter& filters) const {
    const std::vector<std::string> fields{ "id", "username", "picture", "email", "created_at", "updated_at" };
    SQLBuilder qb("users");
    qb.select(fields);
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

    PgResult result = co_await pool_->query(
        qb.str(),
        qb.params(),
        std::chrono::seconds(5)
    );

    std::vector<UserEntity> users;
    users.reserve(result.rows.size());

    for (auto&[columns] : result.rows) {
        UserEntity user;
        user.id = std::stoll(columns[0].data);
        user.username = columns[1].data;
        user.picture = columns[2].is_null
            ? std::nullopt
            : std::make_optional(columns[2].data);
        user.email = columns[3].data;
        user.created_at = parse_pg_timestamp(columns[4].data);
        user.updated_at = parse_pg_timestamp(columns[5].data);
        users.push_back(std::move(user));
    }

    co_return users;
}

net::awaitable<void> UsersRepository::create(UserEntity& entity) const {
    std::vector<std::optional<std::string>> params;
    params.emplace_back(entity.username);
    params.emplace_back(entity.email);
    params.emplace_back(entity.password);

    std::vector<std::string> fields{ "username", "email", "password" };
    SQLBuilder qb("users");
    qb.insert(fields);
    qb.returning("id");

    auto result = co_await pool_->query(
        qb.str(),
        params,
        std::chrono::seconds(5)
    );

    if (!result.rows.empty() && !result.rows[0].columns[0].is_null) {
        entity.id = std::stoll(result.rows[0].columns[0].data);
    }
}

net::awaitable<void> UsersRepository::update(UserEntity& entity) {
    LoggerSingleton::get().info("UsersRepository::update: called", {
        {"id", entity.id},
        {"email", entity.email.value_or("null")},
        {"username", entity.username.value_or("null")},
        {"picture", entity.picture.has_value() ? std::string("passed") : std::string("null")},
    });
    std::vector<std::optional<std::string>> params;
    if (!entity.email->empty()) params.emplace_back(entity.email);
    if (!entity.username->empty()) params.emplace_back(entity.username);
    if (!entity.password->empty()) params.emplace_back(entity.password);
    if (!entity.picture->empty()) params.emplace_back(entity.picture);
    LoggerSingleton::get().debug("UsersRepository::update: emplaced fields to update", {
        {"size", params.size()}
    });

    co_return;
}
