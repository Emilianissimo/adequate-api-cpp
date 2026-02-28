//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_RENDERERINTERFACE_H
#define BEAST_API_RENDERERINTERFACE_H

#include <nlohmann/json.hpp>
#include "core/request/Request.h"
#include "core/http/interfaces/HttpInterface.h"

using json = nlohmann::json;

class RendererInterface
{
public:
    virtual ~RendererInterface() {}

    virtual Response render(
        const Request& request,
        http::status status,
        const json& body,
        bool keepAlive = false,
        std::unordered_map<http::field, std::string> additionalHeaders = {},
        int dumpIndent = -1
    ) = 0;

    virtual Response error(
        const Request& request,
        http::status status,
        std::string_view message,
        bool keepAlive = false,
        std::unordered_map<http::field, std::string> additionalHeaders = {},
        int dumpIndent = -1
    ) = 0;
};

#endif //BEAST_API_RENDERERINTERFACE_H
