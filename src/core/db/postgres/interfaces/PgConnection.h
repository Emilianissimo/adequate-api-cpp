#pragma once

#include "core/db/postgres/interfaces/PgTypes.h"
#include <boost/asio.hpp>
#include <libpq-fe.h>
#include <string>
#include <optional>
#include <chrono>
#include <unordered_set>

namespace net = boost::asio;

class PgConnection {
public:
    PgConnection(net::any_io_executor executor, std::string dsn);
    ~PgConnection();

    net::awaitable<void> connect();
    net::awaitable<bool> ping();

    net::awaitable<PgResult> execParams(
        std::string_view sql,
        const std::vector<std::optional<std::string>>& params,
        std::chrono::steady_clock::duration timeout
    );

    net::awaitable<PgResult> execPrepared(
        std::string_view stmtName,
        std::string_view sqlIfPrepareNeeded,
        const std::vector<std::optional<std::string>>& params,
        std::chrono::steady_clock::duration timeout
    );

    /// Transactions
    net::awaitable<void> begin(std::chrono::steady_clock::duration timeout);
    net::awaitable<void> commit(std::chrono::steady_clock::duration timeout);
    net::awaitable<void> rollback(std::chrono::steady_clock::duration timeout);

    [[nodiscard]] bool healthy() const noexcept;

private:
    net::any_io_executor executor_;
    std::string dsn_;
    PGconn* conn_{nullptr};
    std::optional<net::posix::stream_descriptor> stream_descriptor_;
    std::unordered_set<std::string> prepared_; // per-connection cache

    net::awaitable<void> wait_readable(std::chrono::steady_clock::time_point deadline);
    net::awaitable<void> wait_writable(std::chrono::steady_clock::time_point deadline);
    net::awaitable<void> ensure_connected();
    void close();

    net::awaitable<PgResult> runQueryWithTimeout(
        std::function<int()> sendFn,
        std::chrono::steady_clock::duration timeout
    );

    static PgResult buildResult(const PGresult* r);
    void cancel() const noexcept; // best-effort PQcancel
};
