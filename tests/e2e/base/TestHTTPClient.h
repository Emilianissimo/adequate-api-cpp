//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_TESTHTTPCLIENT_H
#define BEAST_API_TESTHTTPCLIENT_H

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;

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
        std::multimap<std::string, std::string> headers;
    };

    struct RawResponse {
        http::status status;
        json body;
        std::string rawBody;
        std::map<std::string, std::string> headers;
    };

    class TestHttpClient
    {
    public:
        explicit TestHttpClient(std::string host, std::string port = "80")
            : host_(std::move(host))
            , port_(std::move(port))
            , resolver_(ioc_)
            , stream_(ioc_) {}

        HttpResponse get(const std::string& target, const std::multimap<std::string, std::string>& headers = {})
        {
            return request(http::verb::get, target, "", merge(headers));
        }

        HttpResponse postJson(const std::string& target,
                          const std::string& jsonBody,
                          const std::multimap<std::string, std::string>& headers = {})
        {
            auto h = headers;
            h.emplace("content-type", "application/json");
            return request(http::verb::post, target, jsonBody, merge(h));
        }

        void setDefaultHeader(std::string k, std::string v) {
            defaultHeaders_.emplace(std::move(k), std::move(v));
        }

        HttpResponse del(
            const std::string& target,
            const std::multimap<std::string, std::string>& headers = {}
        )
        {
            return request(http::verb::delete_, target, "", merge(headers));
        }

        RawResponse patchMultipart(
            const std::string& target,
            const std::string& boundary,
            const std::string& multipartBody
        )
        {
            std::multimap<std::string, std::string> h;
            h.emplace("content-type", "multipart/form-data; boundary=" + boundary);
            h.emplace("accept", "application/json");

            auto [status, body, headers] = request(http::verb::patch, target, multipartBody, merge(h));

            RawResponse out;
            out.status = status;
            out.rawBody = std::move(body);

            // headers -> map (lowercase keys)
            for (auto& [k, v] : headers) {
                std::string lk = k;
                std::ranges::transform(lk, lk.begin(), [](const unsigned char c){ return static_cast<char>(std::tolower(c)); });
                out.headers[lk] = v;
            }

            try {
                if (auto it = out.headers.find("content-type");
                    it != out.headers.end() && it->second.find("application/json") != std::string::npos)
                {
                    out.body = json::parse(out.rawBody);
                }
            } catch (...) {
                out.body = json();
            }

            return out;
        }


        RawResponse getRaw(
            const std::string& path,
            const std::vector<std::pair<std::string,std::string>>& extraHeaders = {})
        {
            std::multimap<std::string, std::string> h;
            for (const auto& [k, v] : extraHeaders) h.emplace(k, v);

            auto [status, body, headers] = request(http::verb::get, path, "", merge(h));

            RawResponse out;
            out.status = status;
            out.rawBody = std::move(body);

            for (auto& [k, v] : headers) {
                std::string lk = k;
                std::ranges::transform(lk, lk.begin(), [](const unsigned char c){ return static_cast<char>(std::tolower(c)); });
                out.headers[lk] = v;
            }

            // JSON best-effort
            try {
                if (const auto it = out.headers.find("content-type");
                    it != out.headers.end() && it->second.find("application/json") != std::string::npos)
                {
                    out.body = json::parse(out.rawBody);
                }
            } catch (...) {
                out.body = json();
            }

            return out;
        }

    private:
        std::string host_;
        std::string port_;

        net::io_context ioc_;
        tcp::resolver resolver_;
        beast::tcp_stream stream_;

        std::multimap<std::string,std::string> defaultHeaders_;

        std::multimap<std::string,std::string> merge(const std::multimap<std::string,std::string>& h) const {
            auto out = defaultHeaders_;
            out.insert(h.begin(), h.end());
            return out;
        }

        HttpResponse request(
            http::verb method,
            const std::string& target,
            const std::string& body,
            const std::multimap<std::string, std::string>& headers
        )
        {
            auto const results = resolver_.resolve(host_, port_);
            stream_.connect(results);

            http::request<http::string_body> req{method, target, 11};
            req.set(http::field::host, host_);
            req.set(http::field::user_agent, "e2e-test-client");
            req.keep_alive(false);

            for (auto& [k, v] : headers) req.set(k, v);

            req.body() = body;
            req.prepare_payload();

            http::write(stream_, req);

            beast::flat_buffer buffer;
            http::response<http::string_body> res;
            http::read(stream_, buffer, res);

            beast::error_code ec;
            stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

            HttpResponse out;
            out.status = res.result();
            out.body   = res.body();

            for (auto const& f : res.base()) {
                out.headers.emplace(std::string(f.name_string()), std::string(f.value()));
            }

            return out;
        }
    };
} // namespace test::http

#endif //BEAST_API_TESTHTTPCLIENT_H
