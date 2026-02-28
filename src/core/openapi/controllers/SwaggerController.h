//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_SWAGGERCONTROLLER_H
#define BEAST_API_SWAGGERCONTROLLER_H

#include "core/http/interfaces/HttpInterface.h"
#include "../../renderers/json/JsonRenderer.h"
#include "core/http/ResponseTypes.h"
#include <fstream>
#include <utility>

class SwaggerController {
public:
    explicit SwaggerController(
        std::filesystem::path rootPath
    ) : rootPath_(std::move(rootPath)) {};

    [[nodiscard]] net::awaitable<Outcome> index(const Request& request) const;

private:
    std::filesystem::path rootPath_;
};

#endif //BEAST_API_SWAGGERCONTROLLER_H
