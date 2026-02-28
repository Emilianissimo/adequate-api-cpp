//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_SWAGGERSERVICE_H
#define BEAST_API_SWAGGERSERVICE_H

#include <fstream>
#include "core/openapi/OpenApiBuilder.h"

class SwaggerService
{
public:
    /// Sync, works only on build
    static void generate(const Router& router, const std::filesystem::path& rootPath)
    {
        const Json documentation = OpenApiBuilder::build(router);

        std::ofstream file(rootPath / "public/openapi/swagger.json");
        file << documentation.dump(4);
        file.close();
    }
};

#endif //BEAST_API_SWAGGERSERVICE_H
