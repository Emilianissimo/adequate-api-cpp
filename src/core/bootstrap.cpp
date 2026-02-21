#include "core/Bootstrap.h"
#include "core/routers/Router.h"
#include "core/db/postgres/interfaces/PgPool.h"
#include "core/loggers/LoggerSingleton.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <memory>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = net::ip::tcp;
using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;
using RawRequest = http::request<http::string_body>;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
    tcp::socket socket_;
    Router& router_;
    const EnvConfig& env_;
    beast::flat_buffer buffer_;

public:
    HttpSession(tcp::socket&& socket, Router& router, const EnvConfig& env)
        : socket_(std::move(socket))
        , router_(router)
        , env_(env)
    {
    }

    net::awaitable<void> run() {
        auto self = shared_from_this();

        try {
            for (;;) {
                http::request_parser<http::string_body> parser;
                parser.body_limit(env_.file_upload_limit_size);

                co_await http::async_read(socket_, buffer_, parser, use_awaitable);

                RawRequest raw = parser.release();
                Request req(std::move(raw), env_);

                Response res = co_await router_.dispatch(std::move(req), env_);

                bool keep = res.keep_alive();

                co_await http::async_write(socket_, res, use_awaitable);

                if (!keep) {
                    beast::error_code ec;
                    socket_.shutdown(tcp::socket::shutdown_send, ec);
                    break;
                }
            }
        } catch (const boost::system::system_error& se) {
            LoggerSingleton::get().warn("Session error: " + se.code().message());
        } catch (const std::exception& ex) {
            LoggerSingleton::get().error("Session exception: " + std::string(ex.what()));
        }

        co_return;
    }

private:
    net::awaitable<void> sendError(http::status status, std::string msg) {
        Response res{status, 11};
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(false);
        res.body() = std::move(msg);
        res.prepare_payload();

        boost::system::error_code ec;
        co_await http::async_write(socket_, res, net::redirect_error(use_awaitable, ec));
        socket_.shutdown(tcp::socket::shutdown_send, ec);
    }
};

// ==========================================
// Listener
// ==========================================
static awaitable<void> listener(const std::string& host, const uint16_t port, Router& router, const EnvConfig& env) {
    const auto exec = co_await net::this_coro::executor;
    tcp::endpoint ep{ net::ip::make_address(host), port };
    tcp::acceptor acc(exec, ep);

    for (;;) {
        tcp::socket sock(exec);
        co_await acc.async_accept(sock, use_awaitable);

        // Создаем сессию через make_shared (выделяет память одним куском, меньше фрагментации)
        auto session = std::make_shared<HttpSession>(std::move(sock), router, env);

        // Запускаем через co_spawn.
        // session->run() держит self, так что session не умрет.
        co_spawn(exec, session->run(), detached);
    }
}

int Bootstrap::run(net::io_context& ioc, const EnvConfig& env, Router& router) {
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