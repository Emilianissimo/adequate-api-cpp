#include "core/routers/Router.h"
#include "core/errors/Errors.h"
#include "core/renderers/JsonRenderer.h"
#include <boost/beast/http.hpp>
#include "core/loggers/LoggerSingleton.h"
#include <algorithm>
#include <sstream>
#include <type_traits>

using namespace std;

Router& Router::add(http::verb method, std::string path, RouteFn fn) {
    for (auto& entry : table_) {
        if (entry.original == path) {
            entry.methods[method] = std::move(fn);
            return *this;
        }
    }
    auto entry = compileRoute(path);
    entry.methods[method] = std::move(fn);
    table_.push_back(std::move(entry));
    return *this;
}

Router& Router::use(std::shared_ptr<MiddlewareInterface> middleware) {
    global_middlewares_.push_back(std::move(middleware));
    return *this;
}

Router& Router::use(std::string prefix, std::shared_ptr<MiddlewareInterface> mw) {
    scoped_middlewares_.push_back({std::move(prefix), std::move(mw)});
    return *this;
}

std::vector<std::shared_ptr<MiddlewareInterface>> Router::collectMiddlewaresFor(const std::string& path) const {
    std::vector<std::shared_ptr<MiddlewareInterface>> out = global_middlewares_;
    for (auto &s : scoped_middlewares_) {
        if (path.rfind(s.prefix, 0) == 0) {
            out.push_back(s.middleware);
        }
    }
    return out;
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

Outcome Router::make404(const Request& request) {
    JsonResult result{json{{"error", "Not found"}}, http::status::not_found, false};
    return JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
}

Outcome Router::make405(const Request& request, const MethodMap& mm) {
    JsonResult result{ json{{"error", "Method Not Allowed"}}, http::status::method_not_allowed, false };
    Response tmp = JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
    tmp.set(http::field::allow, buildAllowHeader(mm));
    return tmp;
}

Outcome Router::make500(const Request& request, std::string& err) {
    JsonResult result{json{{"error", err}}, http::status::internal_server_error, false};
    return JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
}

Outcome Router::makeOptionsAllow(const Request& request, const MethodMap& mm) {
    JsonResult result{ json{{"allow", buildAllowHeader(mm)}}, http::status::ok, true };
    Response tmp = JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
    tmp.set(http::field::allow, buildAllowHeader(mm));
    return tmp;
}

/// Exact-match: cutting query string
std::string Router::normalizeTarget(const Request& request) {
    std::string target_path = std::string(request.target());
    if (auto pos = target_path.find('?'); pos != std::string::npos) target_path.resize(pos);
    return target_path;
}

net::awaitable<Outcome> Router::runChain(Request& request, RouteFn leaf, std::vector<std::shared_ptr<MiddlewareInterface>>& middlewares) const {
    // Compiling chain of middleware to one next()
    using Next = MiddlewareInterface::Next;
    
    Next next = [leaf](Request& r) -> net::awaitable<Outcome> {
        co_return co_await leaf(r);
    };

    // Оборачиваем в обратном порядке: последний mw близко к leaf
    for (auto middleware = middlewares.rbegin(); middleware != middlewares.rend(); ++middleware) {
        auto& middleware_ = *middleware;
        Next prev = std::move(next);
        next = [middleware_, prev](Request& request_) -> net::awaitable<Outcome> {
            co_return co_await middleware_->handle(request_, prev);
        };
    }

    co_return co_await next(request);
}

net::awaitable<Response> Router::runAfter(const Request& request, Response&& response, std::vector<std::shared_ptr<MiddlewareInterface>>& middlewares) const {
    Response result = std::move(response);
    for (auto& middleware : middlewares) {
        result = co_await middleware->after(request, std::move(result));
    }
    co_return result;
}

Response Router::render(const Request& request, Outcome&& outcome) const {
    return std::visit([&](auto&& v) -> Response {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, Response>) {
            return std::move(v);
        } else {
            return JsonRenderer::jsonResponse(
                request,
                v.status,
                v.body,
                v.keepAlive,
                v.dumpIndent
            );
        }
    }, std::move(outcome));
}

/// Compiling route with
/// Static segment fallback -> Static segment (without any params) and Dynamic param pattern
Router::RouteEntry Router::compileRoute(const std::string& tmpl)
{
    auto escape_segment = [](const std::string& s) -> std::string {
        static const std::string specials = R"(\.^$|()[]{}*+?!)";
        std::string out; out.reserve(s.size()*2);
        for (char c : s) {
            if (specials.find(c) != std::string::npos) out.push_back('\\');
            out.push_back(c);
        }
        return out;
    };

    std::string regexStr = "^";
    std::vector<std::string> params;

    std::istringstream ss(tmpl);
    std::string seg;

    while (std::getline(ss, seg, '/'))
    {
        if (seg.empty()) continue;
        regexStr += "/";
        if (seg.front() == '{' && seg.back() == '}')
        {
            std::string name = seg.substr(1, seg.size() - 2);
            params.emplace_back(std::move(name));
            regexStr += "([^/]+)";
        } else
        {
            regexStr += escape_segment(seg);
        }
    }
    regexStr += "$";

    return {std::regex(regexStr), params, {}, tmpl};
}

net::awaitable<Response> Router::dispatch(Request& request) const {
    LoggerSingleton::get().info("Router::dispatch: called", {
        {"method", request.method()},
        {"target", request.target()}
    });
    int error_code = 0;
    std::string error_msg;
    const auto path = this->normalizeTarget(request);

    auto middlewares = collectMiddlewaresFor(path);

    const MethodMap* methods = nullptr;
    for (auto& entry : table_)
    {
        std::smatch match;
        if (std::regex_match(path, match, entry.regex))
        {
            std::unordered_map<std::string, std::string> params;
            for (size_t i = 0; i < entry.paramNames.size(); ++i)
            {
                params[entry.paramNames[i]] = match[i + 1];
            }
            request.path_params = std::move(params);

            methods = &entry.methods;
            break;
        }
    }

    if (!methods)
        co_return co_await this->runAfter(request, this->render(request, this->make404(request)), middlewares);

    // Auto-HEAD: if no HEAD, but GET exists — return GET without body
    if (request.method() == http::verb::head && !methods->count(http::verb::head) && methods->count(http::verb::get)) {
        Outcome outcome = co_await runChain(request, methods->at(http::verb::get), middlewares);
        Response response = this->render(request, std::move(outcome));
        response.body().clear();
        response.set(http::field::content_length, "0");
        for (auto& middleware : middlewares) {
            response = co_await middleware->after(request, std::move(response));
        };
        co_return response;
    }

     // Auto-OPTIONS: if no OPTIONS — return Allow
    if (request.method() == http::verb::options && !methods->count(http::verb::options)) {
        co_return co_await this->runAfter(request, this->render(request, this->makeOptionsAllow(request, *methods)), middlewares);
    }

    auto itMeth = methods->find(request.method());
    if (itMeth == methods->end()) {
        co_return co_await this->runAfter(request, this->render(request, this->make405(request, *methods)), middlewares);
    }

    try {
        auto fn = itMeth->second;
        auto outcome = co_await this->runChain(request, itMeth->second, middlewares);
        Response response = this->render(request, std::move(outcome));
        co_return co_await this->runAfter(request, std::move(response), middlewares);
    } catch (const DbError& e) {
        error_code = 500;
        error_msg = e.what() && *e.what() ? e.what() : "Database error";
    } catch (const std::exception& e) {
        error_code = 500;
        error_msg = e.what() && *e.what() ? e.what() : "Unexpected error";
    }

    if (error_code == 500) {
        co_return co_await this->runAfter(
            request,
            this->render(request, this->make500(request, error_msg)),
            middlewares
        );
    }

    error_msg = "Unexpected error";
    co_return co_await this->runAfter(
        request,
        this->render(request, this->make500(request, error_msg)),
        middlewares
    );
}
