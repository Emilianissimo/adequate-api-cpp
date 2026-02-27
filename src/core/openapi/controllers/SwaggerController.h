//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_SWAGGERCONTROLLER_H
#define BEAST_API_SWAGGERCONTROLLER_H

#include "core/http/interfaces/HttpInterface.h"
#include "core/renderers/JsonRenderer.h"
#include "core/http/ResponseTypes.h"
#include <fstream>
#include <utility>

class SwaggerController {
public:
    explicit SwaggerController(
        std::filesystem::path rootPath,
        std::filesystem::path mediaPath
    ) : rootPath_(std::move(rootPath)), mediaPath_(std::move(mediaPath)) {};

    [[nodiscard]] net::awaitable<Outcome> index(const Request& request) const;

private:
    std::filesystem::path rootPath_;
    std::filesystem::path mediaPath_;
};

#endif //BEAST_API_SWAGGERCONTROLLER_H
