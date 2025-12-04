#pragma once
#include <boost/beast/http.hpp>
#include <boost/url.hpp>
#include <unordered_map>
#include <nlohmann/json.hpp>

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
            nlohmann::json::parse(req_.body());
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Invalid JSON: ") + e.what());
        }
    }

    void parseMultipart() {
        std::string ct = content_type();
        if (ct.find("multipart/form-data") == std::string::npos)
            throw std::runtime_error("Content-Type is not multipart");

        // boundary=...
        auto pos = ct.find("boundary=");
        if (pos == std::string::npos)
            throw std::runtime_error("multipart: boundary not found");

        std::string boundary = "--" + ct.substr(pos + 9);
        std::string bodyStr = req_.body();

        MultipartForm form;

        size_t start = bodyStr.find(boundary);
        if (start == std::string::npos)
            throw std::runtime_error("multipart: boundary missing in body");

        start += boundary.size();

        while (true) {
            size_t headerEnd = bodyStr.find("\r\n\r\n", start);
            if (headerEnd == std::string::npos) break;

            std::string headerBlock = bodyStr.substr(start, headerEnd - start);

            size_t namePos = headerBlock.find("name=\"");
            if (namePos == std::string::npos) break;

            namePos += 6;
            size_t nameEnd = headerBlock.find("\"", namePos);
            std::string name = headerBlock.substr(namePos, nameEnd - namePos);

            size_t filenamePos = headerBlock.find("filename=\"");
            std::string filename;
            if (filenamePos != std::string::npos) {
                filenamePos += 10;
                size_t filenameEnd = headerBlock.find("\"", filenamePos);
                filename = headerBlock.substr(filenamePos, filenameEnd - filenamePos);
            }

            size_t contentStart = headerEnd + 4;
            size_t contentEnd = bodyStr.find(boundary, contentStart);
            if (contentEnd == std::string::npos) break;

            std::string dataStr = bodyStr.substr(contentStart, contentEnd - contentStart - 2);

            if (!filename.empty()) {
                MultipartPart part;
                part.name = name;
                part.filename = filename;
                part.data.assign(dataStr.begin(), dataStr.end());
                form.files[name] = std::move(part);
            } else {
                form.fields[name] = dataStr;
            }

            start = contentEnd + boundary.size();
            if (bodyStr.substr(start, 2) == "--") break; // end
        }

        multipart_ = std::move(form);
    }

private:
    RawRequest req_;
    std::optional<MultipartForm> multipart_;
};
