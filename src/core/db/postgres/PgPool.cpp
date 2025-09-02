#include "core/db/postgres/interfaces/PgPool.h"
#include "core/db/postgres/interfaces/PgConnection.h"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/steady_timer.hpp>
#include <deque>
#include <stdexcept>

namespace net = boost::asio;

PgPool::PgPool(net::any_io_executor executor, std::string dsn, std::size_t size)
    : executor_(std::move(executor))
    , strand_(net::make_strand(executor_))
    , dsn_(std::move(dsn))
    , size_(size)
    , channel_(strand_, /*capacity*/ static_cast<unsigned>(size ? size : 1)) {}

net::awaitable<PgPool::Lease> PgPool::acquire() {
    auto conn = co_await make_or_wait();
    // RAII release closure
    Lease lease{
        conn,
        [this, weak = std::weak_ptr<PgConnection>(conn)]() {
            if (auto c = weak.lock()) {
                this->release(c);
            }
        }
    };
    co_return lease;
}

void PgPool::shutdown() {
    net::dispatch(strand_, [this] {
        stopping_ = true;
        if (created_at_ >= idle_.size()) {
            created_at_ -= idle_.size();
        } else {
            created_at_ = 0;
        }
        for (auto& c : idle_) {
            c.reset(); // dtor PgConnection → PQfinish
        }
        idle_.clear();
    });
}

net::awaitable<PgResult> PgPool::query(
    std::string_view sql,
    const std::vector<std::optional<std::string>>& params,
    std::chrono::steady_clock::duration timeout
) {
    auto lease = co_await acquire();
    try {
        auto res = co_await lease.connection->execParams(sql, params, timeout);
        lease.release();
        co_return res;
    } catch (...) {
        // if connection is broken, drop it (release() will health-check)
        lease.release();
        throw;
    }
}


net::awaitable<std::shared_ptr<PgConnection>> PgPool::make_or_wait() {
    co_await net::dispatch(strand_, net::use_awaitable);

    if (stopping_) {
        throw std::runtime_error("PgPool is shutting down");
    }

    // 1) Reuse idle if any
    if (!idle_.empty()) {
        auto c = idle_.back();
        idle_.pop_back();
        co_return c;
    }

    // 2) Create new if capacity allows
    if (created_at_ < size_) {
        auto c = std::make_shared<PgConnection>(executor_, dsn_);
        ++created_at_;
        // connect now (may throw)
        co_await c->connect();
        co_return c;
    }

    // 3) Wait on channel for a returned connection
    auto tup = co_await channel_.async_receive(
        net::as_tuple(net::use_awaitable_t<net::any_io_executor>{})
    );

    auto ec  = std::get<0>(tup);
    auto got = std::get<1>(tup);

    if (ec) {
        throw boost::system::system_error(ec);
    }
    co_return got;
}

void PgPool::release(std::shared_ptr<PgConnection> connection) {
    if (!connection) return;
    net::dispatch(strand_, [this, c = std::move(connection)]() mutable {
        if (stopping_) {
            if (created_at_) --created_at_;
            return; // drop
        }
        if (!c->healthy()) {
            if (created_at_) --created_at_;
            return; // drop
        }
        if (channel_.try_send(boost::system::error_code{}, c)) {
            return; // gave back to awaited one
        }
        idle_.push_back(std::move(c)); // park
    });
}
