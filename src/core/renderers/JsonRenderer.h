#pragma once

#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace http = boost::beast::http;
using Request  = http::request<http::string_body>;
using Response = http::response<http::string_body>;
using json     = nlohmann::json;

struct JsonRenderer{
    static Response jsonResponse(
        const Request& request,
        http::status status,
        const json& body,
        bool keepAlive = false,
        int dumpIndent = -1
    );
    
    static Response jsonError(
        const Request& request,
        http::status status,
        std::string_view message,
        bool keepAlive = false,
        int dumpIndent = -1
    );
};
