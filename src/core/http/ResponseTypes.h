#pragma once
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include <variant>

namespace http = boost::beast::http;

using Request  = http::request<http::string_body>;
using Response = http::response<http::string_body>;
using json     = nlohmann::json;

struct JsonResult {
    json         body;
    http::status status     = http::status::ok;
    bool         keepAlive  = true;
    int          dumpIndent = -1;

    JsonResult() = default;

    // Universal constructor — auto json(T)
    template<class T>
    JsonResult(
        T&& v,
        http::status s=http::status::ok,
        bool ka=true,
        int indent=-1
    ) : body(nlohmann::json(std::forward<T>(v))), status(s), keepAlive(ka), dumpIndent(indent) {}
};

using Outcome = std::variant<Response, JsonResult>;
