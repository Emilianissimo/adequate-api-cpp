//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_SWAGGERCONTROLLER_H
#define BEAST_API_SWAGGERCONTROLLER_H

#include "core/http/interfaces/HttpInterface.h"
#include "core/renderers/JsonRenderer.h"
#include "core/http/ResponseTypes.h"
#include <fstream>
#include <sstream>

class SwaggerController {
public:
    explicit SwaggerController(const std::filesystem::path& rootPath) : rootPath_(rootPath) {};

    net::awaitable<Outcome> index(Request& request) const
    {
        std::ifstream file(rootPath_ / "swagger.json");

        if (!file) {
            co_return JsonResult{
                json{{"error", "Swagger file not generated"}},
                http::status::not_found,
                false
            };
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        Response response;
        response.result(http::status::ok);
        response.set(http::field::content_type, "application/json");
        response.body() = buffer.str();
        response.prepare_payload();

        co_return response;
    }

private:
    std::filesystem::path rootPath_;
};

#endif //BEAST_API_SWAGGERCONTROLLER_H
