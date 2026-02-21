//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_TESTHTTPCLIENT_H
#define BEAST_API_TESTHTTPCLIENT_H

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <string>

namespace test::http
{
    namespace net = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;
    using tcp = net::ip::tcp;

    struct HttpResponse
    {
        http::status status;
        std::string body;
    };

    class TestHttpClient
    {
    public:
        TestHttpClient(std::string host, std::string port = "80")
            : host_(std::move(host))
            , port_(std::move(port))
            , resolver_(ioc_)
            , stream_(ioc_) {}

        HttpResponse get(std::string target)
        {
            auto const results = resolver_.resolve(host_, port_);
            stream_.connect(results);

            http::request<http::empty_body> req{http::verb::get, target, 11};
            req.set(http::field::host, host_);
            req.set(http::field::user_agent, "e2e-test-client");

            http::write(stream_, req);

            beast::flat_buffer buffer;
            http::response<http::string_body> res;
            http::read(stream_, buffer, res);

            stream_.socket().shutdown(tcp::socket::shutdown_both);

            return {
            res.result(),
                res.body()
            };
        }

    private:
        std::string host_;
        std::string port_;

        net::io_context ioc_;
        tcp::resolver resolver_;
        beast::tcp_stream stream_;
    };
} // namespace test::http

#endif //BEAST_API_TESTHTTPCLIENT_H
