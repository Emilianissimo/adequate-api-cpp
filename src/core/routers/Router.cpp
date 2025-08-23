#include "Router.h"
#include <boost/beast/http.hpp>
#include <boost/beast/core/string.hpp>
#include <algorithm>
#include <sstream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

Router& Router::add(http::verb method, std::string path, RouteFn fn) {
    table_[std::move(path)][method] = std::move(fn);
    return *this;
}

void Router::use(std::shared_ptr<MiddlewareInterface> middleware) {
    middlewares_.push_back(std::move(middleware));
}

static std::string buildAllowHeader(const Router::MethodMap& mm) {
    struct Item { http::verb verb; const char* name; };
    static constexpr Item items[]{
        {http::verb::get,     "GET"},
        {http::verb::head,    "HEAD"},
        {http::verb::post,    "POST"},
        {http::verb::put,     "PUT"},
        {http::verb::patch,   "PATCH"},
        {http::verb::delete_, "DELETE"},
        {http::verb::options, "OPTIONS"},
    };
    std::string allow;
    for (auto& item : items) {
        if (mm.count(item.verb)) {
            if (!allow.empty()) allow += ", ";
            allow += item.name;
        }
    }
    return allow;
}

Response Router::make404(const Request& request) {
    json body = {
        {"error", "Not found"}
    };

    Response response{http::status::not_found, request.version()};
    response.set(http::field::content_type, "application/json");
    response.keep_alive(false);
    response.body() = body.dump();
    response.prepare_payload();
    return response;
}

Response Router::make405(const Request& request, const MethodMap& mm) {
    json body = {
        {"error", "Method not allowed"}
    };

    Response response{http::status::method_not_allowed, request.version()};
    response.set(http::field::content_type, "application/json");
    response.set(http::field::allow, buildAllowHeader(mm));
    response.keep_alive(false);
    response.body() = body.dump();
    response.prepare_payload();
    return response;
}

Response Router::makeOptionsAllow(const Request& request, const MethodMap& mm) {
    Response response{http::status::ok, request.version()};
    response.set(http::field::allow, buildAllowHeader(mm));
    response.keep_alive(true);
    response.prepare_payload();
    return response;
}

std::string Router::normalizeTarget(const Request& request) {
    // exact-match: cutting query string
    std::string target_path = std::string(request.target());
    if (auto pos = target_path.find('?'); pos != std::string::npos) target_path.resize(pos);
    return target_path;
}

net::awaitable<Response> Router::runChain(Request& request, RouteFn leaf) const {
    // Compiling chain of middleware to one next()
    using NextFn = std::function<net::awaitable<Response>(Request&)>;
    NextFn next = [leaf](Request& r) -> net::awaitable<Response> {
        co_return co_await leaf(r);
    };

    // Оборачиваем в обратном порядке: последний mw близко к leaf
    for (auto middleware = middlewares_.rbegin(); middleware != middlewares_.rend(); ++middleware) {
        auto& middleware_ = *middleware;
        NextFn prev = std::move(next);
        next = [middleware_, prev](Request& request_) -> net::awaitable<Response> {
            co_return co_await middleware_->handle(request_, prev);
        };
    }

    co_return co_await next(request);
}

net::awaitable<Response> Router::dispatch(Request& request) const {
    const auto path = normalizeTarget(request);
    auto itPath = table_.find(path);
    if (itPath == table_.end()) {
        co_return make404(request);
    }

    const auto& methods = itPath->second;

    // Auto-HEAD: if no HEAD, but GET exists — return GET without body
    if (request.method() == http::verb::head && !methods.count(http::verb::head) && methods.count(http::verb::get)) {
        auto fn = methods.at(http::verb::get);
        auto response = co_await runChain(request, fn);
        response.body().clear();
        response.set(http::field::content_length, "0");
        co_return response;
    }

    // Auto-OPTIONS: if no OPTIONS — return Allow
    if (request.method() == http::verb::options && !methods.count(http::verb::options)) {
        co_return makeOptionsAllow(request, methods);
    }

    auto itMeth = methods.find(request.method());
    if (itMeth == methods.end()) {
        co_return make405(request, methods);
    }

    auto fn = itMeth->second;
    co_return co_await runChain(request, fn);
}
