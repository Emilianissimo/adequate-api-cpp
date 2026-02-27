//
// Created by user on 27.02.2026.
//

#ifndef BEAST_API_HTMLRENDERER_H
#define BEAST_API_HTMLRENDERER_H

#include <boost/beast/http.hpp>

#include "core/http/interfaces/HttpInterface.h"
#include "core/request/Request.h"


class HtmlRenderer
{
public:
    static Response file(
        const Request& request,
        const std::filesystem::path& filePath,
        http::status status = http::status::ok
    );
};


#endif //BEAST_API_HTMLRENDERER_H
