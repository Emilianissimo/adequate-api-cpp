//
// Created by user on 27.02.2026.
//

#include <fstream>
#include <sstream>
#include <string>
#include "HtmlRenderer.h"

Response HtmlRenderer::file(
    const Request& request,
    const std::filesystem::path& filePath,
    http::status status
) {
    std::string filePath_ = filePath.string();
    std::ifstream f(filePath, std::ios::binary);
    if (!f) {
        Response res{http::status::not_found, request.raw().version()};
        res.set(http::field::content_type, "text/plain; charset=utf-8");
        res.keep_alive(request.keep_alive());
        res.body() = "HTML template not found: " + filePath_;
        res.prepare_payload();
        return res;
    }

    std::stringstream ss;
    ss << f.rdbuf();

    Response res{status, request.raw().version()};
    res.set(http::field::content_type, "text/html; charset=utf-8");
    res.keep_alive(request.keep_alive());
    res.body() = ss.str();
    res.prepare_payload();
    return res;
}
