#pragma once
#include <boost/beast/http.hpp>
#include <boost/url.hpp>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace http = boost::beast::http;

class Request {
public:
    using RawRequest = http::request<http::string_body>;

    explicit Request(RawRequest req) : req_(std::move(req)) {}

    const RawRequest& raw() const { return req_; }
    RawRequest& raw() { return req_; }

    const std::string& body() const { return req_.body(); }


    std::unordered_map<std::string, std::string> query() const {
        std::unordered_map<std::string, std::string> params;
        
        boost::urls::url_view url_view_(this->target());
        for (auto qp : url_view_.params()) {
            params.emplace(std::string(qp.key), std::string(qp.value));
        }
        return params;
    }

    nlohmann::json json() const {
        try {
            return nlohmann::json::parse(req_.body());
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Invalid JSON: ") + e.what());
        }
    }

    const auto& headers() const { return req_; }

    http::verb method() const { return req_.method(); }

    inline unsigned int version() const { return req_.version(); };

    std::string target() const { return std::string(req_.target()); }

    inline bool keep_alive() const { return req_.keep_alive(); };

private:
    RawRequest req_;
};
