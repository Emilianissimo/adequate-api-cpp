//
// Created by user on 28.02.2026.
//

#ifndef BEAST_API_RESPONSE_H
#define BEAST_API_RESPONSE_H

#include "core/http/interfaces/HttpInterface.h"

class Response final : public RawResponse
{
public:
    using RawResponse::RawResponse;

    static Response redirect(
        const unsigned version,
        const bool keepAlive,
        const std::string_view location,
        const http::status code = http::status::found
    ) {
        Response res{code, version};
        res.set(http::field::location, location);
        res.set(http::field::content_type, "text/plain; charset=utf-8");
        res.keep_alive(keepAlive);
        res.body() = "Redirecting to " + std::string(location);
        res.prepare_payload();
        return res;
    }
};

#endif //BEAST_API_RESPONSE_H
