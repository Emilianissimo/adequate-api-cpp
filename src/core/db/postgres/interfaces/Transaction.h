#pragma once

#include "core/db/postgres/interfaces/PgPool.h"
#include <chrono>

struct Transaction {
    PgPool::Lease Lease;
    std::chrono::steady_clock::duration timeout{};
    
    static net::awaitable<Transaction> begin(PgPool& pool, std::chrono::steady_clock::duration timeout);
    [[nodiscard]] net::awaitable<void> commit() const;
    [[nodiscard]] net::awaitable<void> rollback() const;
};
