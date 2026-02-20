#pragma once

#include "core/db/postgres/interfaces/PgConnection.h"
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <memory>
#include <vector>

class PgPool {
public:
    PgPool(net::any_io_executor executor, std::string dsn, std::size_t size);

    // Circuit braker for DB 503
    enum class BreakerState { Closed, Open, HalfOpen };

    /// As far as we use strand_, we can guarantee that breaker will be serialized and we don't need mutex to block it
    struct CircuitBreaker
    {
        // Closed by default, do not return 503
        BreakerState state = BreakerState::Closed;

        std::size_t consecutive_failures = 0;
        std::size_t failure_threshold = 5; // OPEN - after 5 in a row

        std::chrono::steady_clock::duration open_duration{std::chrono::seconds(3)};
        std::chrono::steady_clock::time_point open_until{};

        bool half_open_probe_inflight = false;

        bool allow(std::chrono::steady_clock::time_point now) {
            if (state == BreakerState::Closed) return true;

            if (state == BreakerState::Open) {
                if (now < open_until) return false;
                // run probe
                state = BreakerState::HalfOpen;
                half_open_probe_inflight = false;
            }

            // HalfOpen: access only for 1 probe
            if (half_open_probe_inflight) return false;
            half_open_probe_inflight = true;
            return true;
        }

        void on_success() {
            state = BreakerState::Closed;
            consecutive_failures = 0;
            half_open_probe_inflight = false;
        }

        void on_failure(std::chrono::steady_clock::time_point now) {
            if (state == BreakerState::HalfOpen) {
                // probe failed -> OPEN
                state = BreakerState::Open;
                open_until = now + open_duration;
                consecutive_failures = failure_threshold;
                half_open_probe_inflight = false;
                return;
            }

            // Closed/Open: increasing
            ++consecutive_failures;
            if (consecutive_failures >= failure_threshold) {
                state = BreakerState::Open;
                open_until = now + open_duration;
                half_open_probe_inflight = false;
            }
        }
    };

    struct Lease {
        std::shared_ptr<PgConnection> connection;
        std::function<void()> release; // RAII return to pool
    };

    net::awaitable<Lease> acquire();
    /// Gracefully kill
    void shutdown();

    net::awaitable<PgResult> query(
        const std::string& sql,
        const std::vector<std::optional<std::string>>& params,
        std::chrono::steady_clock::duration timeout
    );

private:
    net::any_io_executor executor_;
    net::strand<net::any_io_executor> strand_;
    std::string dsn_;
    const std::size_t size_;
    std::vector<std::shared_ptr<PgConnection>> idle_;
    std::size_t created_at_{0};

    /// Queue of awaiting objects
    net::experimental::channel<void(boost::system::error_code, std::shared_ptr<PgConnection>)> channel_;
    bool stopping_{false};
    net::awaitable<std::shared_ptr<PgConnection>> make_or_wait();
    void release(std::shared_ptr<PgConnection> connection);

    CircuitBreaker breaker_;
};
