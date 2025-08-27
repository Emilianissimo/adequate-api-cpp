#pragma once

#include "core/db/postgres/interfaces/PgPool.h"
#include <chrono>

struct Transaction {
    PgPool::Lease Lease;
    std::chrono::steady_clock::duration timeout;
    
    static net::awaitable<Transaction> begin(PgPool& pool, std::chrono::steady_clock::duration timeout);
    net::awaitable<void> commit();
    net::awaitable<void> rollback();
};
