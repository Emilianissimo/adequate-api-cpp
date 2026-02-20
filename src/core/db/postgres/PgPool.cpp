#include "core/db/postgres/interfaces/PgPool.h"
#include "core/db/postgres/interfaces/PgConnection.h"

#include <boost/asio/as_tuple.hpp>
#include <deque>
#include <stdexcept>

#include "core/errors/Errors.h"

namespace net = boost::asio;

PgPool::PgPool(net::any_io_executor executor, std::string dsn, const std::size_t size)
    : executor_(std::move(executor))
    , strand_(net::make_strand(executor_))
    , dsn_(std::move(dsn))
    , size_(size)
    , channel_(strand_, /*capacity*/ static_cast<unsigned>(size ? size : 1)) {}

net::awaitable<PgPool::Lease> PgPool::acquire() {
    const std::shared_ptr<PgConnection> conn = co_await make_or_wait();
    // RAII release closure
    Lease lease{
        conn,
        [this, weak = std::weak_ptr<PgConnection>(conn)]() {
            if (const std::shared_ptr<PgConnection> c = weak.lock()) {
                this->release(c);
            }
        }
    };
    co_return lease;
}

void PgPool::shutdown() {
    net::dispatch(strand_, [this] {
        stopping_ = true;
        channel_.close();

        for (std::shared_ptr<PgConnection>& c : idle_) {
            c.reset(); // dtor PgConnection → PQfinish
        }
        idle_.clear();
        created_at_ = 0;
    });
}

net::awaitable<PgResult> PgPool::query(
    const std::string& sql,
    const std::vector<std::optional<std::string>>& params,
    const std::chrono::steady_clock::duration timeout
) {
    auto [connection, release] = co_await acquire();
    try {
        auto res = co_await connection->execParams(sql, params, timeout);
        breaker_.on_success();
        release();
        co_return res;
    } catch (const DbError&) {
        // if connection is broken, drop it (release() will health-check)
        // Breaker shouldn't react on SQL errors
        release();
        throw;
    } catch (...)
    {
        // Infra errors, breaker should run failure protocol
        breaker_.on_failure(std::chrono::steady_clock::now());
        release();
        throw;
    }
}


net::awaitable<std::shared_ptr<PgConnection>> PgPool::make_or_wait() {
    co_await net::dispatch(strand_, net::use_awaitable);

    if (stopping_) {
        throw std::runtime_error("PgPool is shutting down");
    }

    auto now = std::chrono::steady_clock::now();
    if (!breaker_.allow(now)) {
        throw std::runtime_error("Database temporarily unavailable (circuit open)");
    }

    // 1) Reuse idle if any
    if (!idle_.empty()) {
        std::shared_ptr<PgConnection> c = idle_.back();
        idle_.pop_back();
        co_return c;
    }

    // 2) Create new if capacity allows
    if (created_at_ < size_) {
        std::shared_ptr<PgConnection> c = std::make_shared<PgConnection>(strand_, dsn_);

        try
        {
            co_await c->connect();   // if it is failed -> infra error
            ++created_at_;
            breaker_.on_success();
            co_return c;
        } catch (...) {
            breaker_.on_failure(std::chrono::steady_clock::now());
            // created at shouldn't be touched then
            throw;
        }
    }

    // 3) Wait on channel for a returned connection
    auto [ec, got] = co_await channel_.async_receive(net::as_tuple(net::use_awaitable));
    if (ec) throw boost::system::system_error(ec);
    co_return got;
}

void PgPool::release(std::shared_ptr<PgConnection> connection) {
    if (!connection) return;
    net::dispatch(strand_, [this, c = std::move(connection)]() mutable {
        if (stopping_) {
            if (created_at_) --created_at_;
            c.reset();
            return; // drop
        }
        if (!c->healthy()) {
            if (created_at_) --created_at_;
            c.reset();
            return; // drop
        }
        if (channel_.try_send(boost::system::error_code{}, c)) {
            return; // gave back to awaited one
        }
        idle_.push_back(std::move(c)); // park
    });
}
