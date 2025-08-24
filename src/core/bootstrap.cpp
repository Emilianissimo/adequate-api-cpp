#include "core/Bootstrap.h"
#include "core/routers/Router.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <thread>


namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = net::ip::tcp;
using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;

static awaitable<void> session(tcp::socket socket, Router& router) {
    beast::flat_buffer buffer;
    try {
        for (;;) {
            Request req;
            co_await http::async_read(socket, buffer, req, use_awaitable);

            Response res = co_await router.dispatch(req);
            bool keep = res.keep_alive();

            co_await http::async_write(socket, res, use_awaitable);

            if (!keep) {
                beast::error_code ec;
                socket.shutdown(tcp::socket::shutdown_send, ec);
                break;
            }
        }
    } catch (const std::exception&) {
        beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_send, ec);
    }
    co_return;
}

static awaitable<void> listener(const std::string host, uint16_t port, Router& router) {
    auto exec = co_await net::this_coro::executor;

    tcp::endpoint ep{ net::ip::make_address(host), port };
    tcp::acceptor acc(exec, ep);

    for (;;) {
        tcp::socket sock(exec);
        co_await acc.async_accept(sock, use_awaitable);
        co_spawn(exec, session(std::move(sock), router), detached);
    }
}

int Bootstrap::run(const EnvConfig& env, Router& router) {
    try {
        net::io_context ioc{ static_cast<int>(std::max(1u, std::thread::hardware_concurrency())) };

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ ioc.stop(); });

        co_spawn(ioc, listener(env.host, env.port, router), detached);
        std::cout << "Listening on http://" << env.host << ":" << env.port << "\n";

        ioc.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
