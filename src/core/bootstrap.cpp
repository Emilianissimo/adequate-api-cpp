#include "core/Bootstrap.h"
#include "core/routers/Router.h"
#include "core/db/postgres/interfaces/PgPool.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = net::ip::tcp;
using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;
using RawRequest = http::request<http::string_body>;

static awaitable<void> session(tcp::socket socket, Router& router, const EnvConfig& env) {
    bool shouldWriteError = false;
    Response errorResponse;
    try {
        beast::flat_buffer buffer;
        for (;;) {
            http::request_parser<http::string_body> parser;
            // Body limit
            parser.body_limit(env.file_upload_limit_size);

            co_await http::async_read(socket, buffer, parser, use_awaitable);

            RawRequest raw = parser.release();

            Request req(std::move(raw), env);
            Response res = co_await router.dispatch(req);
            bool keep = res.keep_alive();

            co_await http::async_write(socket, res, use_awaitable);

            if (!keep) {
                beast::error_code ec;
                socket.shutdown(tcp::socket::shutdown_send, ec);
                break;
            }
        }
    } catch (const boost::system::system_error& se) {
        if (se.code() == http::error::body_limit) {
            shouldWriteError = true;

            errorResponse = {http::status::payload_too_large, 11};
            errorResponse.set(http::field::content_type, "text/plain");
            errorResponse.keep_alive(false);              // важно
            errorResponse.body() = "Payload too large";
            errorResponse.prepare_payload();
        }
    } catch (const std::exception& ex) {}
    if (shouldWriteError) {
        boost::system::error_code ec;
        co_await http::async_write(socket, errorResponse,
                                  boost::asio::redirect_error(use_awaitable, ec));
    }
    {
        beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_send, ec);
    }
    co_return;
}

static awaitable<void> listener(const std::string& host, const uint16_t port, Router& router, const EnvConfig& env) {
    const auto exec = co_await net::this_coro::executor;

    tcp::endpoint ep{ net::ip::make_address(host), port };
    tcp::acceptor acc(exec, ep);

    for (;;) {
        tcp::socket sock(exec);
        co_await acc.async_accept(sock, use_awaitable);
        co_spawn(exec, session(std::move(sock), router, env), detached);
    }
}

int Bootstrap::run(boost::asio::io_context& ioc, const EnvConfig& env, Router& router) {
    try {
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ ioc.stop(); });

        co_spawn(ioc, listener(env.host, env.port, router, env), detached);
        std::cout << "Listening on http://" << env.host << ":" << env.port << "\n";

        ioc.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
