#pragma once

#include "core/db/postgres/interfaces/PgConnection.h"
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <memory>
#include <vector>

namespace net = boost::asio;

class PgPool {
public:
    PgPool(net::any_io_executor executor, std::string dsn, std::size_t size);

    struct Lease {
        std::shared_ptr<PgConnection> connection;
        std::function<void()> release; // RAII return to pool
    };

    net::awaitable<Lease> acquire();
    void shutdown(); // Gracefull kill

    net::awaitable<PgResult> query(
        std::string_view sql,
        const std::vector<std::optional<std::string>>& params,
        std::chrono::steady_clock::duration timeout
    );

private:
    net::any_io_executor executor_;
    std::string dsn_;
    const std::size_t size_;
    std::vector<std::shared_ptr<PgConnection>> idle_;
    std::size_t created_at_{0};

    // Queue of awating objects
    net::experimental::channel<void(std::shared_ptr<PgConnection>)> channel_;
    bool stopping_{false};
    net::awaitable<std::shared_ptr<PgConnection>> make_or_wait();
    void release(std::shared_ptr<PgConnection> connection);
};
