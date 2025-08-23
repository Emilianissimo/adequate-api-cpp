#pragma once

#include "HttpInterface.h"
#include "MiddlewareInterface.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <memory>

class Router {
public:
    using RouteFn = std::function<net::awaitable<Response>(Request&)>;
    using MethodMap = std::unordered_map<http::verb, RouteFn>;
    
    Router& add(http::verb method, std::string path, RouteFn fn);

    Router& get(std::string path, RouteFn fn) { return add(http::verb::get, std::move(path), std::move(fn)); };
    Router& head(std::string path, RouteFn fn) { return add(http::verb::head, std::move(path), std::move(fn)); };
    Router& options(std::string path, RouteFn fn) { return add(http::verb::options, std::move(path), std::move(fn)); };
    Router& post(std::string path, RouteFn fn) { return add(http::verb::post, std::move(path), std::move(fn)); };
    Router& put(std::string path, RouteFn fn) { return add(http::verb::put, std::move(path), std::move(fn)); };
    Router& patch(std::string path, RouteFn fn) { return add(http::verb::patch, std::move(path), std::move(fn)); };
    Router& delete_(std::string path, RouteFn fn) { return add(http::verb::delete_, std::move(path), std::move(fn)); };

    void use(std::shared_ptr<MiddlewareInterface> middleware);

    net::awaitable<Response> dispatch(Request& request) const;

private:
    std::unordered_map<std::string, MethodMap> table_;
    std::vector<std::shared_ptr<MiddlewareInterface>> middlewares_;

    static std::string normalizeTarget(const Request& request);
    static Response make404(const Request& request);
    static Response make405(const Request& request, const MethodMap& mm);
    static Response makeOptionsAllow(const Request& request, const MethodMap& mm);

    net::awaitable<Response> runChain(Request& request, RouteFn leaf) const;
};
