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
    
    Router& add(
        http::verb method,
        const std::string& path,
        RouteFn fn,
        const std::vector<std::string>& allowedContentTypes
    );

    Router& get(std::string path, RouteFn fn) { return add(http::verb::get, std::move(path), std::move(fn), {}); };
    Router& head(std::string path, RouteFn fn) { return add(http::verb::head, std::move(path), std::move(fn), {}); };
    Router& options(std::string path, RouteFn fn) { return add(http::verb::options, std::move(path), std::move(fn), {}); };
    Router& post(std::string path, RouteFn fn, const std::vector<std::string> &allowedContentTypes = {"application/json"}) {
        return add(http::verb::post, std::move(path), std::move(fn), allowedContentTypes);
    };
    Router& put(std::string path, RouteFn fn, const std::vector<std::string> &allowedContentTypes = {"application/json"}) {
        return add(http::verb::put, std::move(path), std::move(fn), allowedContentTypes);
    };
    Router& patch(std::string path, RouteFn fn, const std::vector<std::string> &allowedContentTypes = {"application/json"}) {
        return add(http::verb::patch, std::move(path), std::move(fn), allowedContentTypes);
    };
    Router& delete_(std::string path, RouteFn fn, const std::vector<std::string> &allowedContentTypes = {"application/json"}) {
        return add(http::verb::delete_, std::move(path), std::move(fn), allowedContentTypes);
    };

    /// Use globally
    Router& use(std::shared_ptr<MiddlewareInterface> middleware);
    /// Use locally (scoped)
    Router& use(std::string pathPrefix, std::shared_ptr<MiddlewareInterface> middleware);

    net::awaitable<Response> dispatch(Request& request) const;

private:
    struct RouteEntry {
        std::regex regex;
        std::vector<std::string> paramNames;
        MethodMap methods;
        std::string original;
        std::unordered_map<http::verb, std::vector<std::string>> allowedContentTypes;
    };
    std::vector<RouteEntry> table_;

    std::vector<std::shared_ptr<MiddlewareInterface>> global_middlewares_;
    struct ScopedMiddlewares { std::string prefix; std::shared_ptr<MiddlewareInterface> middleware; };
    std::vector<ScopedMiddlewares> scoped_middlewares_;

    [[nodiscard]] std::vector<std::shared_ptr<MiddlewareInterface>> collectMiddlewaresFor(const std::string& path) const;

    static std::string normalizeTarget(const Request& request);
    static Outcome make400(const Request& request, const std::string& error);
    static Outcome make404(const Request& request);
    static Outcome make405(const Request& request, const MethodMap& mm);
    static Outcome make413(const Request& request, const MethodMap& mm);
    static Outcome make415(const Request& request, const MethodMap& mm);
    static Outcome make500(const Request& request, std::string& err);
    static Outcome makeOptionsAllow(const Request& request, const MethodMap& mm);

    static net::awaitable<Outcome> runChain(Request& request, RouteFn leaf, std::vector<std::shared_ptr<MiddlewareInterface>>& middlewares) ;
    static Response render(const Request& request, Outcome&& outcome) ;
    static net::awaitable<Response> runAfter(const Request& request, Response&& response, const std::vector<std::shared_ptr<MiddlewareInterface>>& middlewares) ;
    static RouteEntry compileRoute(const std::string& tmpl);
};
