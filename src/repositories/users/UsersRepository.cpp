#include "repositories/users/UsersRepository.h"
#include "helpers/DatetimeConverter.h"
#include "core/db/postgres/builder/SQLBuilder.h"
#include "core/loggers/LoggerSingleton.h"
#include "core/errors/Errors.h"
#include "core/http/interfaces/HttpInterface.h"

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

/// Throws validation error on user not found
net::awaitable<UserEntity> UsersRepository::getOne(const UserFilter& filters) const {
    const std::vector<std::string> fields{ "id", "username", "picture", "email", "created_at", "updated_at", "password" };
    SQLBuilder qb("users");
    qb.select(fields);
    if (filters.email.has_value()) {
        qb.where("email", filters.email.value());
    }
    qb.limit(1);
    auto [columns_names, rows] = co_await pool_->query(
        qb.str(),
        qb.params(),
        std::chrono::seconds(5)
    );
    UserEntity user;
    if (rows.empty()) throw ValidationError("user_not_found");
    user.id = std::stoll(rows[0].columns[0].data);
    user.username = rows[0].columns[1].data;
    user.picture = rows[0].columns[2].is_null
        ? std::nullopt
        : std::make_optional(rows[0].columns[2].data);
    user.email = rows[0].columns[3].data;
    user.created_at = parse_pg_timestamp(rows[0].columns[4].data);
    user.updated_at = parse_pg_timestamp(rows[0].columns[5].data);
    user.password = rows[0].columns[6].data;
    co_return user;
}

net::awaitable<bool> UsersRepository::exists(const UserFilter& filters) const {
    SQLBuilder qb("users");
    const std::vector<std::string> fields{ "1"};
    qb.select(fields);
    // NOTE: may be expanded
    if (filters.id.has_value()) {
        qb.where("id", filters.id.value());
    }
    if (filters.email.has_value()) {
        qb.where("email", filters.email.value_or(""));
    }
    qb.exists();
    auto [columns_names, rows] = co_await pool_->query(
       qb.str(),
       qb.params(),
       std::chrono::seconds(5)
    );
    // first and only one exists anyway as a row
    co_return rows[0].columns[0].data == "t";
}

net::awaitable<void> UsersRepository::create(UserEntity& entity) const {
    std::vector<std::optional<std::string>> params;
    params.emplace_back(entity.username);
    params.emplace_back(entity.email);
    params.emplace_back(entity.password);

    const std::vector<std::string> fields{ "username", "email", "password" };
    SQLBuilder qb("users");
    qb.insert(fields);
    qb.returning("id");

    auto result = co_await pool_->query(
        qb.str(),
        // Remember to coordinate params and fields order to ensure SQL build correctly
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
        {"picture", entity.picture.value_or("null")},
    });
    std::vector<std::optional<std::string>> params;
    std::vector<std::string> fields;

    if (entity.email && !entity.email->empty())
    {
        params.emplace_back(entity.email);
        fields.emplace_back("email");
    }
    if (entity.username && !entity.username->empty())
    {
        params.emplace_back(entity.username);
        fields.emplace_back("username");
    }
    if (entity.password && !entity.password->empty())
    {
        params.emplace_back(entity.password);
        fields.emplace_back("password");
    }
    if (entity.picture && !entity.picture->empty())
    {
        params.emplace_back(entity.picture);
        fields.emplace_back("picture");
    };
    LoggerSingleton::get().debug("UsersRepository::update: emplaced fields to update", {
        {"params_size", std::to_string(params.size())},
        {"fields_size", std::to_string(fields.size())}
    });

    SQLBuilder qb("users");
    qb.update(fields);
    // NOTE: remember to add where with id, in other case you will update each row in table!
    // To provide WHERE $3 in this case
    params.emplace_back(std::to_string(entity.id));
    qb.where("id", entity.id);
    qb.returning("id");

    auto result = co_await pool_->query(
        qb.str(),
        // Remember to coordinate params and fields order to ensure SQL build correctly
        params,
        std::chrono::seconds(5)
    );
}
