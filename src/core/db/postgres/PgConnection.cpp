#include "core/db/postgres/interfaces/PgConnection.h"
#include "core/errors/Errors.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>
#include <stdexcept>
#include <unordered_set>

namespace {
    [[noreturn]] void throw_pg_error(PGconn* c, const char* what) {
        const char* em = c ? PQerrorMessage(c) : "No connection";
        std::string message = std::string(what) + ": " + (em ? em : "Unknown error.");
        throw std::runtime_error(message);
    }

    inline bool msg_contains(std::string_view hay, std::string_view needle) {
        return hay.find(needle) != std::string_view::npos;
    }

    inline bool is_already_exists_error(std::string_view err) {
        // Postgres: SQLSTATE 42P05 duplicate_prepared_statement
        return msg_contains(err, "42P05")
            || msg_contains(err, "duplicate_prepared_statement")
            || msg_contains(err, "already exists");
    }

    inline bool is_invalid_prepared_stmt_error(std::string_view err) {
        // Postgres: SQLSTATE 26000 invalid_sql_statement_name
        return msg_contains(err, "26000")
            || msg_contains(err, "invalid prepared statement")
            || msg_contains(err, "invalid_sql_statement_name");
    }
} // namespace

PgConnection::PgConnection(net::any_io_executor executor, std::string dsn) 
    : executor_(std::move(executor)), 
      dsn_(std::move(dsn)) {}

PgConnection::~PgConnection() { close(); }

PgResult PgConnection::buildResult(PGresult* result){
    if (!result) return {};
    PgResult out;

    int nfields = PQnfields(result);
    out.columns_names.reserve(nfields);
    for (int i = 0; i < nfields; ++i){
        out.columns_names.emplace_back(PQfname(result, i) ? PQfname(result, i) : "");
    }

    int nrows = PQntuples(result);
    out.rows.reserve(nrows);
    for (int i = 0; i < nrows; ++i){
        PgRow row;
        row.columns.reserve(nfields);
        for (int j = 0; j < nfields; ++j){
            if (PQgetisnull(result, i, j)) {
                row.columns.push_back(PgValue{std::string{}, true});
            } else {
                const char* val = PQgetvalue(result, i, j);
                int len = PQgetlength(result, i, j);
                row.columns.push_back(PgValue{std::string{val, static_cast<std::size_t>(len)}, false});
            }
        }
        out.rows.push_back(std::move(row));
    }
    return out;
}

bool PgConnection::healthy() const noexcept {
    return conn_ && PQstatus(conn_) == CONNECTION_OK;
}

net::awaitable<void> PgConnection::connect() {
    if (healthy()) co_return;

    close();

    conn_ = PQconnectStart(dsn_.c_str());
    if (!conn_) {
        throw std::runtime_error("PQconnectStart returned nullptr");
    }
    if (PQsetnonblocking(conn_, 1) != 0) {
        throw_pg_error(conn_, "PQsetnonblocking raised an error");
    }

    // Create descriptor from socket
    int fd = PQsocket(conn_);
    if (fd < 0) throw_pg_error(conn_, "PQsocket");
    stream_descriptor_.emplace(executor_, fd);

    // Poll until OK
    while (true) {
        PostgresPollingStatusType st = PQconnectPoll(conn_);
        if (st == PGRES_POLLING_OK) break;
        if (st == PGRES_POLLING_FAILED) {
            throw_pg_error(conn_, "PQconnectPoll failed");
        }

        // wait for IO readiness
        if (st == PGRES_POLLING_READING) {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
            co_await wait_readable(deadline);
        } else if (st == PGRES_POLLING_WRITING) {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
            co_await wait_writable(deadline);
        } else {
            // continue
        }
    }

    prepared_.clear();
    co_return;
}

net::awaitable<bool> PgConnection::ping() {
    try {
        co_await ensure_connected();
        auto result = co_await execParams("SELECT 1", {}, std::chrono::seconds(3));
        // cheap round-trip
        (void)result;
        co_return true;
    } catch (...) {
        co_return false;
    }
}

net::awaitable<void> PgConnection::begin(std::chrono::steady_clock::duration timeout) {
    auto r = co_await execParams("BEGIN", {}, timeout);
    (void)r;
    co_return;
}

net::awaitable<void> PgConnection::commit(std::chrono::steady_clock::duration timeout) {
    auto r = co_await execParams("COMMIT", {}, timeout);
    (void)r;
    co_return;
}

net::awaitable<void> PgConnection::rollback(std::chrono::steady_clock::duration timeout) {
    auto r = co_await execParams("ROLLBACK", {}, timeout);
    (void)r;
    co_return;
}

net::awaitable<PgResult> PgConnection::execParams(
    std::string_view sql,
    const std::vector<std::optional<std::string>>& params,
    std::chrono::steady_clock::duration timeout
) {
    co_await ensure_connected();

    std::vector<const char*> values; values.reserve(params.size());
    std::vector<int> formats; formats.reserve(params.size());
    std::vector<std::string> storage; storage.reserve(params.size());

    for (auto const& param : params) {
        if (param.has_value()) {
            storage.push_back(*param);
            values.push_back(storage.back().c_str());
        } else {
            values.push_back(nullptr);
        }
        formats.push_back(0);
    }

    auto send = [&]() -> int {
        return PQsendQueryParams(
            conn_,
            std::string(sql).c_str(),
            static_cast<int>(params.size()),
            nullptr, // infer types
            values.data(),
            nullptr, // lengths for text -> nullptr
            formats.data(), // 0 -> text
            0 // result format text
        );
    };

    co_return co_await runQueryWithTimeout(send, timeout);
}

net::awaitable<PgResult> PgConnection::execPrepared(
    std::string_view stmtName,
    std::string_view sqlIfPrepareNeeded,
    const std::vector<std::optional<std::string>>& params,
    std::chrono::steady_clock::duration timeout
) {
    co_await ensure_connected();

    // Lazy per-connection cache of prepared names
    const std::string name(stmtName);
    const std::string sql_if_needed(sqlIfPrepareNeeded);

    auto do_prepare = [&]() -> net::awaitable<void>{
        if (prepared_.count(name)) co_return;

        auto send_prepare = [&]() -> int {
            return PQsendPrepare(
                conn_,
                name.c_str(),
                sql_if_needed.c_str(),
                static_cast<int>(params.size()),
                nullptr // infer types
            );
        };

        try {
            (void)co_await runQueryWithTimeout(send_prepare, timeout);
            prepared_.insert(name);
            co_return;
        } catch (const std::exception& e) {
            const std::string msg = e.what();
            if (is_already_exists_error(msg)) {
                prepared_.insert(name);
                co_return;
            }
            // any other error throw
            throw;
        }
    };

    auto build_param_arrays = [&](std::vector<const char*>& values,
                                  std::vector<int>& formats,
                                  std::vector<std::string>& storage) {
        values.clear(); formats.clear(); storage.clear();
        values.reserve(params.size());
        formats.reserve(params.size());
        storage.reserve(params.size());

        for (auto const& param : params) {
            if (param.has_value()) {
                storage.push_back(*param);
                values.push_back(storage.back().c_str());
            } else {
                values.push_back(nullptr);
            }
            formats.push_back(0); // text
        }
    };

    co_await do_prepare();
    std::vector<const char*> values;
    std::vector<int>         formats;
    std::vector<std::string> storage;

    build_param_arrays(values, formats, storage);

    auto send_exec = [&]() -> int {
        return PQsendQueryPrepared(
            conn_,
            name.c_str(),
            static_cast<int>(params.size()),
            values.data(),
            nullptr, // lengths=nullptr (text)
            formats.data(), // all text
            0 // result text
        );
    };

    bool retry_needed = false;
    PgResult result;

    try {
        result = co_await runQueryWithTimeout(send_exec, timeout);
    } catch (const std::exception& e) {
        if (is_invalid_prepared_stmt_error(e.what())) {
            retry_needed = true;
        } else {
            throw;
        }
    }

    if (retry_needed) {
        prepared_.erase(name);
        co_await do_prepare(); 
        co_return co_await runQueryWithTimeout(send_exec, timeout);
    }

    co_return result;
}

net::awaitable<void> PgConnection::ensure_connected() {
    if (!healthy()) {
        co_await connect();
    }
    co_return;
}

void PgConnection::close() {
    if (stream_descriptor_.has_value()) {
        boost::system::error_code ec;
        stream_descriptor_->cancel(ec);
        stream_descriptor_->release(); // does not close underlying fd (libpq owns it)
        stream_descriptor_.reset();
    }
    if (conn_) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
}

void PgConnection::cancel() noexcept {
    if (!conn_) return;
    char errbuf[256] = {0};
    if (auto* c = PQgetCancel(conn_)) {
        (void)PQcancel(c, errbuf, sizeof(errbuf)); // best-effort
        PQfreeCancel(c);
    }
}

net::awaitable<void> PgConnection::wait_readable(std::chrono::steady_clock::time_point deadline) {
    if (!stream_descriptor_.has_value()) co_return;

    net::steady_timer timer(executor_);
    timer.expires_at(deadline);

    bool timed_out = false;

    // Arm timeout to cancel the descriptor when it fires
    timer.async_wait([this, &timed_out](const boost::system::error_code& ec) {
        if (!ec) {
            timed_out = true;
            if (stream_descriptor_) {
                boost::system::error_code ignore;
                stream_descriptor_->cancel(ignore);
            }
        }
    });

    boost::system::error_code ec;
    co_await stream_descriptor_->async_wait(net::posix::descriptor_base::wait_read, net::redirect_error(net::use_awaitable, ec));

    // Disarm timer
    timer.cancel();

    if (timed_out) {
        throw std::runtime_error("postgres wait_readable timeout");
    }
    if (ec && ec != net::error::operation_aborted) {
        throw std::runtime_error("postgres wait_readable failed: " + ec.message());
    }
    co_return;
}

net::awaitable<void> PgConnection::wait_writable(std::chrono::steady_clock::time_point deadline) {
    if (!stream_descriptor_.has_value()) co_return;

    net::steady_timer timer(executor_);
    timer.expires_at(deadline);

    bool timed_out = false;

    timer.async_wait([this, &timed_out](const boost::system::error_code& ec) {
        if (!ec) {
            timed_out = true;
            if (stream_descriptor_) {
                boost::system::error_code ignore;
                stream_descriptor_->cancel(ignore);
            }
        }
    });

    boost::system::error_code ec;
    co_await stream_descriptor_->async_wait(net::posix::descriptor_base::wait_write, net::redirect_error(net::use_awaitable, ec));

    timer.cancel();

    if (timed_out) {
        throw std::runtime_error("postgres wait_writable timeout");
    }
    if (ec && ec != net::error::operation_aborted) {
        throw std::runtime_error("postgres wait_writable failed: " + ec.message());
    }
    co_return;
}

net::awaitable<PgResult> PgConnection::runQueryWithTimeout(
    std::function<int()> sendFn,
    std::chrono::steady_clock::duration timeout
) {
    auto deadline = std::chrono::steady_clock::now() + timeout;

    try {
        // Send
        if (sendFn() == 0) {
            throw_pg_error(conn_, "PQsend* failed");
        }

        // Flush outgoing
        while (true) {
            int flush_res = PQflush(conn_);
            if (flush_res == 0) break;     // all flushed
            if (flush_res == 1) {
                // need more writable
                co_await wait_writable(deadline);
                continue;
            }
            // error (<0)
            throw_pg_error(conn_, "PQflush error");
        }

        // Receive until done; collect last meaningful result
        PgResult out;
        bool have_result = false;

        for (;;) {
            // Wait for input
            co_await wait_readable(deadline);

            if (PQconsumeInput(conn_) == 0) {
                throw_pg_error(conn_, "PQconsumeInput");
            }

            // If still busy, keep waiting
            if (PQisBusy(conn_) != 0) {
                // loop again
                continue;
            }

            // Drain all results currently available
            while (PGresult* r = PQgetResult(conn_)) {
                ExecStatusType st = PQresultStatus(r);
                if (st == PGRES_TUPLES_OK) {
                    out = buildResult(r);
                    have_result = true;
                } else if (st == PGRES_COMMAND_OK) {
                    // command without rows — keep last status
                    out = {}; // empty
                    have_result = true;
                } else if (st == PGRES_BAD_RESPONSE || st == PGRES_FATAL_ERROR) {
                    std::string em = PQresultErrorMessage(r) ? PQresultErrorMessage(r) : "query error";
                    std::string sqlstate = PQresultErrorField(r, PG_DIAG_SQLSTATE)
                                        ? PQresultErrorField(r, PG_DIAG_SQLSTATE)
                                        : "XXXXX"; // unknown

                    PQclear(r);
                    throw DbError(map_sqlstate(sqlstate), em);
                }
                PQclear(r);
            }

            // When PQgetResult returned nullptr, we're done
            break;
        }

        if (!have_result) {
            // Shouldn't normally happen, but return empty set
            co_return PgResult{};
        }
        co_return out;
    } catch (...) {
        cancel();
        close();
        throw;
    }
}
