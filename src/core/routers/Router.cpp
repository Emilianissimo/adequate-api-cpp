#include "core/routers/Router.h"
#include "core/errors/Errors.h"
#include "core/renderers/JsonRenderer.h"
#include <boost/beast/http.hpp>
#include "core/loggers/LoggerSingleton.h"
#include <algorithm>
#include <ranges>
#include <sstream>
#include <type_traits>

using namespace std;

Router& Router::add(http::verb method, const std::string& path, RouteFn fn, const std::vector<std::string>& allowedContentTypes) {
    for (auto& entry : table_) {
        if (entry.original == path) {
            entry.methods[method] = std::move(fn);
            entry.allowedContentTypes[method] = std::move(allowedContentTypes);
            return *this;
        }
    }
    auto entry = compileRoute(path);
    entry.methods[method] = std::move(fn);
    entry.allowedContentTypes[method] = std::move(allowedContentTypes);
    table_.push_back(std::move(entry));
    return *this;
}

Router& Router::use(std::shared_ptr<MiddlewareInterface> middleware) {
    global_middlewares_.push_back(std::move(middleware));
    return *this;
}

Router& Router::use(std::string pathPrefix, std::shared_ptr<MiddlewareInterface> middleware) {
    scoped_middlewares_.push_back({std::move(pathPrefix), std::move(middleware)});
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
    for (const auto&[verb, name] : items) {
        if (mm.contains(verb)) {
            if (!allow.empty()) allow += ", ";
            allow += name;
        }
    }
    return allow;
}

Outcome Router::make400(const Request& request, const std::string& error) {
    const JsonResult result{json{{"error", error}}, http::status::bad_request, false};
    return JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
}

Outcome Router::make404(const Request& request) {
    const JsonResult result{json{{"error", "Not found"}}, http::status::not_found, false};
    return JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
}

Outcome Router::make405(const Request& request, const MethodMap& mm) {
    const JsonResult result{ json{{"error", "Method Not Allowed"}}, http::status::method_not_allowed, false };
    Response tmp = JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
    tmp.set(http::field::allow, buildAllowHeader(mm));
    return tmp;
}

Outcome Router::make413(const Request& request, const MethodMap& mm) {
    const JsonResult result{json{{"error", "Payload is too large"}}, http::status::payload_too_large, false};
    Response tmp = JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
    tmp.set(http::field::allow, buildAllowHeader(mm));
    return tmp;
}

Outcome Router::make415(const Request& request, const MethodMap& mm) {
    const JsonResult result{json{{"error", "Unsupported media type"}}, http::status::unsupported_media_type, false};
    Response tmp = JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
    tmp.set(http::field::allow, buildAllowHeader(mm));
    return tmp;
}

Outcome Router::make500(const Request& request, std::string& err) {
    const JsonResult result{json{{"error", err}}, http::status::internal_server_error, false};
    return JsonRenderer::jsonResponse(request, result.status, result.body, result.keepAlive, result.dumpIndent);
}

Outcome Router::makeOptionsAllow(const Request& request, const MethodMap& mm) {
    const JsonResult result{ json{{"allow", buildAllowHeader(mm)}}, http::status::ok, true };
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

net::awaitable<Outcome> Router::runChain(Request& request, RouteFn leaf, std::vector<std::shared_ptr<MiddlewareInterface>>& middlewares) {
    // Compiling chain of middleware to one next()
    using Next = MiddlewareInterface::Next;
    
    Next next = [leaf](Request& r) -> net::awaitable<Outcome> {
        co_return co_await leaf(r);
    };

    for (auto & middleware_ : std::ranges::reverse_view(middlewares)) {
        Next prev = std::move(next);
        next = [middleware_, prev](Request& request_) -> net::awaitable<Outcome> {
            co_return co_await middleware_->handle(request_, prev);
        };
    }

    co_return co_await next(request);
}

net::awaitable<Response> Router::runAfter(const Request& request, Response&& response, const std::vector<std::shared_ptr<MiddlewareInterface>>& middlewares) {
    Response result = std::move(response);
    for (auto& middleware : middlewares) {
        result = co_await middleware->after(request, std::move(result));
    }
    co_return result;
}

Response Router::render(const Request& request, Outcome&& outcome) {
    return std::visit([&]<typename T0>(T0&& v) -> Response {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, Response>) {
            return std::forward<T0>(v);
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
        for (const char c : s) {
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

net::awaitable<Response> Router::dispatch(Request& request, const EnvConfig& env) const {
    LoggerSingleton::get().info("Router::dispatch: called", {
        {"method", request.method()},
        {"target", request.target()}
    });
    int error_code = 0;
    std::string error_msg;
    const auto path = Router::normalizeTarget(request);

    auto middlewares = collectMiddlewaresFor(path);

    const MethodMap* methods = nullptr;
    const RouteEntry* matchedEntry = nullptr;
    for (auto& entry : table_)
    {
        if (std::smatch match; std::regex_match(path, match, entry.regex))
        {
            std::unordered_map<std::string, std::string> params;
            for (size_t i = 0; i < entry.paramNames.size(); ++i)
            {
                params[entry.paramNames[i]] = match[i + 1];
            }
            request.path_params = std::move(params);

            matchedEntry = &entry;
            methods = &entry.methods;
            break;
        }
    }

    if (!methods)
        co_return co_await Router::runAfter(request, Router::render(request, Router::make404(request)), middlewares);

    // Auto-HEAD: if no HEAD, but GET exists — return GET without body
    if (request.method() == http::verb::head && !methods->contains(http::verb::head) && methods->contains(http::verb::get)) {
        Outcome outcome = co_await runChain(request, methods->at(http::verb::get), middlewares);
        Response response = Router::render(request, std::move(outcome));
        response.body().clear();
        response.set(http::field::content_length, "0");
        for (auto& middleware : middlewares) {
            response = co_await middleware->after(request, std::move(response));
        };
        co_return response;
    }

     // Auto-OPTIONS: if no OPTIONS — return Allow
    if (request.method() == http::verb::options && !methods->count(http::verb::options)) {
        co_return co_await Router::runAfter(request, Router::render(request, Router::makeOptionsAllow(request, *methods)), middlewares);
    }

    auto itMeth = methods->find(request.method());
    if (itMeth == methods->end()) {
        co_return co_await Router::runAfter(request, Router::render(request, Router::make405(request, *methods)), middlewares);
    }

    std::vector<std::string> allowed;

    if (matchedEntry && matchedEntry->allowedContentTypes.contains(request.method())) {
        allowed = matchedEntry->allowedContentTypes.at(request.method());
    }

    std::string ct = request.content_type();

    if (!allowed.empty() && ct.empty()) {
        co_return co_await Router::runAfter(
            request,
            Router::render(request, make415(request, *methods)),
            middlewares
        );
    }

    if (!allowed.empty()) {
        bool ok = false;
        for (const auto& t : allowed) {
            if (ct.find(t) != std::string::npos) {
                ok = true;
                break;
            }
        }

        if (!ok) {
            co_return co_await Router::runAfter(
                request,
                Router::render(request, make415(request, *methods)),
                middlewares
            );
        }
    }

    // DoS protection
    if (request.raw().body().size() > env.file_upload_limit_size) {
        co_return co_await Router::runAfter(
            request,
            Router::render(request, make413(request, *methods)),
            middlewares
        );
    }

    std::string bodyContentTypeError;

    LoggerSingleton::get().debug("Router::dispatch: Request content-type", {
        {"ct", ct},
        {"size", (int)request.raw().body().size()}
    });

    try {
        if (ct.find("application/json") != std::string::npos) {
            request.ensureJsonValid();
        }
        else if (ct.find("multipart/form-data") != std::string::npos) {
            request.parseMultipart();
        }
    }
    catch (const std::exception& e) {
        bodyContentTypeError = e.what();
    }

    if (!bodyContentTypeError.empty()) {
        co_return co_await Router::runAfter(
            request,
            Router::render(request, make400(request, bodyContentTypeError)),
            middlewares
        );
    }

    try {
        auto fn = itMeth->second;
        auto outcome = co_await Router::runChain(request, itMeth->second, middlewares);
        Response response = Router::render(request, std::move(outcome));
        co_return co_await Router::runAfter(request, std::move(response), middlewares);
    } catch (const DbError& e) {
        error_code = 500;
        error_msg = e.what() && *e.what() ? e.what() : "Database error";
    } catch (const std::exception& e) {
        error_code = 500;
        error_msg = e.what() && *e.what() ? e.what() : "Unexpected error";
    }

    co_return co_await Router::runAfter(
        request,
        Router::render(request, Router::make500(request, error_msg)),
        middlewares
    );
}
