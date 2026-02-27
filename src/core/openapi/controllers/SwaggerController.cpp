#include <sstream>

#include "SwaggerController.h"

#include "core/openapi/filters/SwaggerFilter.h"
#include "core/renderers/HtmlRenderer.h"

net::awaitable<Outcome> SwaggerController::index(const Request& request) const{
    SwaggerFilter filters;
    filters.parseRequestQuery(request.query());

    if (filters.json)
    {
        std::ifstream file(rootPath_ / mediaPath_ / "swagger.json");

        if (!file) {
            co_return JsonResult{
                json{{"error", "Swagger file not generated"}},
                http::status::not_found,
                false
            };
        }
        std::stringstream buffer;
        buffer << file.rdbuf();

        auto body = nlohmann::json::parse(buffer.str());
        JsonResult result{
            body,
            http::status::ok,
            request.keep_alive()
        };
        co_return result;
    }

    co_return HtmlRenderer::file(request, rootPath_ / "src/core/openapi/templates/swagger.html");
}
//
// Created by user on 27.02.2026.
//
