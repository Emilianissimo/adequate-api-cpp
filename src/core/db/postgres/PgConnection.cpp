#include "core/db/postgres/interfaces/PgConnection.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>
#include <stdexcept>
#include <unordered_set>

namespace {
    PgResult BuildResultFromPg(PGresult* result){
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
            for (int j = 0; i < nfields; ++j){
                if (PQgetisnull(result, i, j)) {
                    row.columns.push_back(PgValue{std::string{}, true});
                } else {
                    const char* val = PQgetvalue(result, i, j);
                    int len = PQgetlength(result, i, j);
                    row.columns.push_back(PgValue{std::string{val, len}, false});
                }
            }
            out.rows.push_back(std::move(row));
        }
        return out;
    }
 
    [[noreturn]] void throw_pg_error(PGconn* c, const char* what) {
        const char* em = c ? PQerrorMessage(c) : "No connection";
        std::string message = std::string(what) + ": " + (em ? em : "Unknown error.");
        throw std::runtime_error(message);
    }
}

class PgConnectionImplState {
public:
    std::unordered_set<std::string> prepared_names;
};

PgConnection::PgConnection(net::any_io_executor executor, std::string dsn) 
    : executor_(std::move(executor)), 
      dsn_(std::move(dsn)) {}

PgConnection::~PgConnection() { close(); }

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


