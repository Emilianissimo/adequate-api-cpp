#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = net::ip::tcp;
using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;

struct Env {
    std::string host = "0.0.0.0";
    uint16_t port = 8080;
    static Env from_env() {
        Env e;
        if (const char* h = std::getenv("APP_HOST"); h && *h) e.host = h;
        if (const char* p = std::getenv("APP_PORT"); p && *p) e.port = static_cast<uint16_t>(std::stoi(p));
        return e;
    }
};

http::response<http::string_body>
handle_request(const http::request<http::string_body>& req)
{
    if (req.method() == http::verb::get && req.target() == "/health") {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "beast-coawait");
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = R"({"status":"alive"})";
        res.prepare_payload();
        return res;
    }

    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::server, "beast-coawait");
    res.set(http::field::content_type, "text/plain");
    res.keep_alive(false);
    res.body() = "Not found";
    res.prepare_payload();
    return res;
}

awaitable<void> session(tcp::socket socket)
{
    beast::flat_buffer buffer;
    try {
        for (;;) {
            http::request<http::string_body> req;
            co_await http::async_read(socket, buffer, req, use_awaitable);

            auto res = handle_request(req);
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

awaitable<void> listener(const std::string& host, uint16_t port)
{
    auto exec = co_await net::this_coro::executor;
    tcp::endpoint ep{net::ip::make_address(host), port};
    tcp::acceptor acc(exec, ep);

    for (;;) {
        tcp::socket sock(exec);
        co_await acc.async_accept(sock, use_awaitable);
        co_spawn(exec, session(std::move(sock)), detached);
    }
}

int main()
{
    try {
        Env env = Env::from_env();
        net::io_context ioc{static_cast<int>(std::max(1u, std::thread::hardware_concurrency()))};

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ ioc.stop(); });

        co_spawn(ioc, listener(env.host, env.port), detached);
        std::cout << "Listening on http://" << env.host << ":" << env.port << "\n";
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
