#pragma once

#include "core/interfaces/HttpInterface.h"
#include "core/request/Request.h"
#include "core/interfaces/MiddlewareInterface.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <regex>

class Router {
public:
    using RouteFn = std::function<net::awaitable<Outcome>(Request&)>;
    using MethodMap = std::unordered_map<http::verb, RouteFn>;
    
    Router& add(http::verb method, std::string path, RouteFn fn);

    Router& get(std::string path, RouteFn fn) { return add(http::verb::get, std::move(path), std::move(fn)); };
    Router& head(std::string path, RouteFn fn) { return add(http::verb::head, std::move(path), std::move(fn)); };
    Router& options(std::string path, RouteFn fn) { return add(http::verb::options, std::move(path), std::move(fn)); };
    Router& post(std::string path, RouteFn fn) { return add(http::verb::post, std::move(path), std::move(fn)); };
    Router& put(std::string path, RouteFn fn) { return add(http::verb::put, std::move(path), std::move(fn)); };
    Router& patch(std::string path, RouteFn fn) { return add(http::verb::patch, std::move(path), std::move(fn)); };
    Router& delete_(std::string path, RouteFn fn) { return add(http::verb::delete_, std::move(path), std::move(fn)); };

    Router& use(std::shared_ptr<MiddlewareInterface> middleware); // Use globally
    Router& use(std::string pathPrefix, std::shared_ptr<MiddlewareInterface> middleware); // Use locally (scoped)

    net::awaitable<Response> dispatch(Request& request) const;

private:
    struct RouteEntry {
        std::regex regex;
        std::vector<std::string> paramNames;
        MethodMap methods;
        std::string original;
    };
    std::vector<RouteEntry> table_;

    std::vector<std::shared_ptr<MiddlewareInterface>> global_middlewares_;
    struct ScopedMiddlewares { std::string prefix; std::shared_ptr<MiddlewareInterface> middleware; };
    std::vector<ScopedMiddlewares> scoped_middlewares_;

    std::vector<std::shared_ptr<MiddlewareInterface>> collectMiddlewaresFor(const std::string& path) const;

    static std::string normalizeTarget(const Request& request);
    static Outcome make404(const Request& request);
    static Outcome make405(const Request& request, const MethodMap& mm);
    static Outcome make500(const Request& request, std::string& err);
    static Outcome makeOptionsAllow(const Request& request, const MethodMap& mm);

    net::awaitable<Outcome> runChain(Request& request, RouteFn leaf, std::vector<std::shared_ptr<MiddlewareInterface>>& middlewares) const;
    Response render(const Request& req, Outcome&& outcome) const;
    net::awaitable<Response> runAfter(const Request& req, Response&& res, std::vector<std::shared_ptr<MiddlewareInterface>>& middlewares) const;
    static RouteEntry compileRoute(const std::string& tmpl);
};
