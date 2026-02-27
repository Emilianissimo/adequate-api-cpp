//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_OPENAPIMETA_H
#define BEAST_API_OPENAPIMETA_H

#include <string>
#include <vector>

struct OpenApiResponseMeta {
    int code;
    std::string description;
    std::string schemaName; // "HealthResponse"
};

struct OpenApiMeta {
    std::string summary;
    std::vector<OpenApiResponseMeta> responses;
};

#endif //BEAST_API_OPENAPIMETA_H
