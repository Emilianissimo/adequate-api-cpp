#include "JsonRenderer.h"

Response JsonRenderer::render(
    const Request& request,
    const http::status status,
    const json& body,
    const bool keepAlive,
    std::unordered_map<http::field, std::string> additionalHeaders,
    const int dumpIndent
)
{
    Response response{status, request.version()};

    for (auto & [field, value] : additionalHeaders)
    {
        response.set(field, value);
    }

    response.set(http::field::server, "beast-coawait");
    response.set(http::field::content_type, "application/json");
    response.keep_alive(keepAlive);

    response.body() = (dumpIndent >= 0) ? body.dump(dumpIndent) : body.dump();

    if (status == http::status::no_content) {
        response.body() = "";
        response.content_length(0);
        response.erase(http::field::content_type);
        return response;
    }

    response.prepare_payload();
    return response;
}

Response JsonRenderer::error(
    const Request& request,
    http::status status,
    const std::string_view message,
    const bool keepAlive,
    std::unordered_map<http::field, std::string> additionalHeaders,
    const int dumpIndent
)
{
    const json body = {
        {"error", std::string(message)},
        {"status", static_cast<int>(status)}
    };
    return render(request, status, body, keepAlive, additionalHeaders, dumpIndent);
}
