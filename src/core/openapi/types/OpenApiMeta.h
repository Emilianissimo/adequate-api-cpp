//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_OPENAPIMETA_H
#define BEAST_API_OPENAPIMETA_H

#include <string>
#include <vector>

struct OpenApiResponseMeta {
    int status;
    std::string description;
    std::optional<std::string> schemaRef;
};

struct OpenApiRequestBody {
    // example: "application/json" or "multipart/form-data"
    std::string contentType;
    std::string schemaRef;
    bool required{true};
};

struct OpenApiMeta {
    std::string summary;
    std::vector<OpenApiResponseMeta> responses;
    nlohmann::json parameters = nlohmann::json::array(); // OpenAPI parameters array
    std::optional<OpenApiRequestBody> requestBody;
    bool authRequired{false};
};

#endif //BEAST_API_OPENAPIMETA_H
