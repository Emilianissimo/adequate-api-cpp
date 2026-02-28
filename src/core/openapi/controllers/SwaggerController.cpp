#include "SwaggerController.h"

#include "core/openapi/filters/SwaggerFilter.h"


net::awaitable<Outcome> SwaggerController::index(const Request& request) const{
    SwaggerFilter filters;
    filters.parseRequestQuery(request.query());
    if (filters.json) {
        co_return Response::redirect(
            request.raw().version(),
            request.keep_alive(),
            "/openapi/swagger.json"
        );
    }

    co_return Response::redirect(
        request.raw().version(),
        request.keep_alive(),
        "/openapi/"
    );
}
//
// Created by user on 27.02.2026.
//
