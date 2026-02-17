#pragma once
#include <boost/beast/http.hpp>
#include <boost/url.hpp>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>

#include "core/configs/EnvConfig.h"
#include "core/loggers/LoggerSingleton.h"
#include "core/multipart/MultipartAdapterFactory.h"

namespace http = boost::beast::http;

struct MultipartPart {
    std::string name;
    std::string filename;
    std::string contentType;
    std::vector<uint8_t> data;
};

struct MultipartForm {
    std::unordered_map<std::string, MultipartPart> files;
    std::unordered_map<std::string, std::string> fields;
};


class Request {
public:
    std::optional<int> user_id;
    std::unordered_map<std::string, std::string> path_params;

    using RawRequest = http::request<http::string_body>;

    std::string host;

    explicit Request(RawRequest req, const EnvConfig& env) : req_(std::move(req)), env_(env)
    {
        if (auto it = req_.find(http::field::host); it != req_.end())
            host = std::string(it->value());
    }

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

    [[nodiscard]] std::string content_type() const {
        const auto it = req_.find(http::field::content_type);
        if (it == req_.end()) return "";
        return std::string(it->value());
    }

    [[nodiscard]] const MultipartForm& multipart() const {
        if (!multipart_) throw std::runtime_error("multipart was not parsed");
        return *multipart_;
    }

    void ensureJsonValid() const {
        try {
            const auto j = nlohmann::json::parse(req_.body());
            (void)j;
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Invalid JSON: ") + e.what());
        }
    }

    void parseMultipart() {
        const auto adapter = MultipartAdapterFactory::create(env_.multipart_adapter);

        auto parts = adapter->parse(content_type(), body());
        MultipartForm form;

        for (auto& p : parts) {
            LoggerSingleton::get().info("Request::parseMultipart", {
                {"name", p.name},
                {"filename", p.filename},
                {"ctype", p.contentType},
                {"size", std::to_string(p.data.size())}
            });
            if (p.filename.empty()) {
                std::string s(p.data.begin(), p.data.end());
                LoggerSingleton::get().debug("Request::parseMultipart: MP field value", {
                    {"name", p.name},
                    {"value_dbg", s.substr(0, 80)}
                });
            }
            if (!p.filename.empty()) {
                MultipartPart file;
                file.name = p.name;
                file.filename = p.filename;
                file.contentType = p.contentType;
                file.data.assign(p.data.begin(), p.data.end());

                form.files[file.name] = std::move(file);
            } else {
                form.fields[p.name] = p.data;
            }
        }

        multipart_ = std::move(form);
    }

private:
    RawRequest req_;
    EnvConfig env_;
    std::optional<MultipartForm> multipart_;
};
