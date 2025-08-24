#include "core/renderers/JsonRenderer.h"

Response JsonRenderer::jsonResponse(
    const Request& request,
    http::status status,
    const json& body,
    bool keepAlive,
    int dumpIndent
)
{
    Response response{status, request.version()};
    response.set(http::field::server, "beast-coawait");
    response.set(http::field::content_type, "application/json");
    response.keep_alive(keepAlive);

    response.body() = (dumpIndent >= 0) ? body.dump(dumpIndent) : body.dump();
    response.prepare_payload();
    return response;
}

Response JsonRenderer::jsonError(
    const Request& request,
    http::status status,
    std::string_view text,
    bool keepAlive,
    int dumpIndent
)
{
    json body = {
        {"error", std::string(text)},
        {"status", static_cast<int>(status)}
    };
    return jsonResponse(request, status, body, keepAlive, dumpIndent);
}
