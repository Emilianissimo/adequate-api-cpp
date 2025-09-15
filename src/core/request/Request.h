#pragma once
#include <boost/beast/http.hpp>
#include <boost/url.hpp>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace http = boost::beast::http;

class Request {
public:
    std::unordered_map<std::string, std::string> path_params;

    using RawRequest = http::request<http::string_body>;

    explicit Request(RawRequest req) : req_(std::move(req)) {}

    [[nodiscard]] const RawRequest& raw() const { return req_; }
    RawRequest& raw() { return req_; }

    [[nodiscard]] const std::string& body() const { return req_.body(); }


    [[nodiscard]] std::unordered_map<std::string, std::string> query() const {
        std::unordered_map<std::string, std::string> params;
        
        boost::urls::url_view url_view_(this->target());
        for (auto qp : url_view_.params()) {
            params.emplace(std::string(qp.key), std::string(qp.value));
        }
        return params;
    }

    [[nodiscard]] nlohmann::json json() const {
        try {
            return nlohmann::json::parse(req_.body());
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Invalid JSON: ") + e.what());
        }
    }

    [[nodiscard]] const auto& headers() const { return req_; }

    [[nodiscard]] http::verb method() const { return req_.method(); }

    [[nodiscard]] inline unsigned int version() const { return req_.version(); };

    [[nodiscard]] std::string target() const { return std::string(req_.target()); }

    [[nodiscard]] inline bool keep_alive() const { return req_.keep_alive(); };

private:
    RawRequest req_;
};
