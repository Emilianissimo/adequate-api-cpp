#include "core/db/postgres/interfaces/Transaction.h"

net::awaitable<Transaction> Transaction::begin(PgPool& pool, std::chrono::steady_clock::duration timeout) {
    auto lease = co_await pool.acquire();
    co_await lease.connection->begin(timeout);
    Transaction tx{
        .Lease = std::move(lease),
        .timeout = timeout
    };
    co_return tx;
}

net::awaitable<void> Transaction::commit() {
    co_await Lease.connection->commit(timeout);
    // return to pool
    Lease.release();
    co_return;
}

net::awaitable<void> Transaction::rollback() {
    try {
        co_await Lease.connection->rollback(timeout);
    } catch (...) {
        // swallow rollback errors — connection likely broken; drop it
    }
    Lease.release();
    co_return;
}
