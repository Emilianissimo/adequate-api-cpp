#pragma once
#include <boost/beast/http.hpp>
#include <boost/asio/awaitable.hpp>

namespace http = boost::beast::http;
namespace net = boost::asio;

using Request = http::request<http::string_body>;
using Response = http::response<http::string_body>;
using Handler = net::awaitable<Response>;
