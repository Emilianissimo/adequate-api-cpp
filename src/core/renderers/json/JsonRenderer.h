#pragma once


#include "core/renderers/interfaces/RendererInterface.h"


class JsonRenderer final : public RendererInterface{

public:
    Response render(
        const Request& request,
        http::status status,
        const json& body,
        bool keepAlive = false,
        std::unordered_map<http::field, std::string> additionalHeaders = {},
        int dumpIndent = -1
    ) override;

    Response error(
        const Request& request,
        http::status status,
        std::string_view message,
        bool keepAlive = false,
        std::unordered_map<http::field, std::string> additionalHeaders = {},
        int dumpIndent = -1
    ) override;
};
