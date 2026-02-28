//
// Created by user on 28.02.2026.
//

#ifndef BEAST_API_OPENAPIRESPONSES_H
#define BEAST_API_OPENAPIRESPONSES_H

#include <string>
#include <vector>
#include <optional>
#include <utility>

#include "OpenApiMeta.h"
#include "core/http/interfaces/HttpInterface.h"

class OpenApiResponses {
public:
    // ---- success
    OpenApiResponses& ok(std::string schemaRef, std::string desc = "OK") & {
        addImpl(static_cast<int>(http::status::ok), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& ok(std::string schemaRef, std::string desc = "OK") && {
        addImpl(static_cast<int>(http::status::ok), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& okArray(const std::string& itemSchema, std::string desc = "OK") & {
        auto arrSchema = itemSchema + "Array";
        addImpl(static_cast<int>(http::status::ok), std::move(desc), std::move(arrSchema));
        return *this;
    }
    OpenApiResponses&& okArray(const std::string itemSchema, std::string desc = "OK") && {
        auto arrSchema = itemSchema + "Array";
        addImpl(static_cast<int>(http::status::ok), std::move(desc), std::move(arrSchema));
        return std::move(*this);
    }

    OpenApiResponses& created(std::string schemaRef, std::string desc = "Created") & {
        addImpl(static_cast<int>(http::status::created), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& created(std::string schemaRef, std::string desc = "Created") && {
        addImpl(static_cast<int>(http::status::created), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& noContent(std::string desc = "No Content") & {
        addNoBodyImpl(static_cast<int>(http::status::no_content), std::move(desc));
        return *this;
    }
    OpenApiResponses&& noContent(std::string desc = "No Content") && {
        addNoBodyImpl(static_cast<int>(http::status::no_content), std::move(desc));
        return std::move(*this);
    }

    // ---- common errors (single)
    OpenApiResponses& badRequest(std::string schemaRef = "ErrorResponse",
                                 std::string desc = "Bad request / validation error") & {
        addImpl(static_cast<int>(http::status::bad_request), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& badRequest(std::string schemaRef = "ErrorResponse",
                                  std::string desc = "Bad request / validation error") && {
        addImpl(static_cast<int>(http::status::bad_request), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& unauthorized(std::string schemaRef = "ErrorResponse",
                                   std::string desc = "Unauthorized") & {
        addImpl(static_cast<int>(http::status::unauthorized), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& unauthorized(std::string schemaRef = "ErrorResponse",
                                    std::string desc = "Unauthorized") && {
        addImpl(static_cast<int>(http::status::unauthorized), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& forbidden(std::string schemaRef = "ErrorResponse",
                                std::string desc = "Forbidden") & {
        addImpl(static_cast<int>(http::status::forbidden), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& forbidden(std::string schemaRef = "ErrorResponse",
                                 std::string desc = "Forbidden") && {
        addImpl(static_cast<int>(http::status::forbidden), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& notFound(std::string schemaRef = "ErrorResponse",
                               std::string desc = "Not found") & {
        addImpl(static_cast<int>(http::status::not_found), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& notFound(std::string schemaRef = "ErrorResponse",
                                std::string desc = "Not found") && {
        addImpl(static_cast<int>(http::status::not_found), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& conflict(std::string schemaRef = "ErrorResponse",
                               std::string desc = "Conflict") & {
        addImpl(static_cast<int>(http::status::conflict), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& conflict(std::string schemaRef = "ErrorResponse",
                                std::string desc = "Conflict") && {
        addImpl(static_cast<int>(http::status::conflict), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& tooLarge(std::string schemaRef = "ErrorResponse",
                               std::string desc = "Payload too large") & {
        addImpl(static_cast<int>(http::status::payload_too_large), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& tooLarge(std::string schemaRef = "ErrorResponse",
                                std::string desc = "Payload too large") && {
        addImpl(static_cast<int>(http::status::payload_too_large), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& unsupportedMedia(std::string schemaRef = "ErrorResponse",
                                       std::string desc = "Unsupported media type") & {
        addImpl(static_cast<int>(http::status::unsupported_media_type), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& unsupportedMedia(std::string schemaRef = "ErrorResponse",
                                        std::string desc = "Unsupported media type") && {
        addImpl(static_cast<int>(http::status::unsupported_media_type), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& unprocessableEntity(std::string schemaRef = "ErrorResponse",
                                          std::string desc = "Unprocessable entity") & {
        addImpl(static_cast<int>(http::status::unprocessable_entity), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& unprocessableEntity(std::string schemaRef = "ErrorResponse",
                                           std::string desc = "Unprocessable entity") && {
        addImpl(static_cast<int>(http::status::unprocessable_entity), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& internal(std::string schemaRef = "ErrorResponse",
                               std::string desc = "Internal server error") & {
        addImpl(static_cast<int>(http::status::internal_server_error), std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& internal(std::string schemaRef = "ErrorResponse",
                                std::string desc = "Internal server error") && {
        addImpl(static_cast<int>(http::status::internal_server_error), std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    // ---- grouped helpers
    OpenApiResponses& withAuth() & {
        // calling impls to avoid value-category issues
        addImpl(static_cast<int>(http::status::unauthorized), "Unauthorized", "ErrorResponse");
        addImpl(static_cast<int>(http::status::forbidden), "Forbidden", "ErrorResponse");
        return *this;
    }
    OpenApiResponses&& withAuth() && {
        addImpl(static_cast<int>(http::status::unauthorized), "Unauthorized", "ErrorResponse");
        addImpl(static_cast<int>(http::status::forbidden), "Forbidden", "ErrorResponse");
        return std::move(*this);
    }

    OpenApiResponses& withBody() & {
        addImpl(static_cast<int>(http::status::bad_request), "Bad request / validation error", "ErrorResponse");
        addImpl(static_cast<int>(http::status::payload_too_large), "Payload too large", "ErrorResponse");
        addImpl(static_cast<int>(http::status::unsupported_media_type), "Unsupported media type", "ErrorResponse");
        addImpl(static_cast<int>(http::status::unprocessable_entity), "Unprocessable entity", "ErrorResponse");
        return *this;
    }
    OpenApiResponses&& withBody() && {
        addImpl(static_cast<int>(http::status::bad_request), "Bad request / validation error", "ErrorResponse");
        addImpl(static_cast<int>(http::status::payload_too_large), "Payload too large", "ErrorResponse");
        addImpl(static_cast<int>(http::status::unsupported_media_type), "Unsupported media type", "ErrorResponse");
        addImpl(static_cast<int>(http::status::unprocessable_entity), "Unprocessable entity", "ErrorResponse");
        return std::move(*this);
    }

    OpenApiResponses& withAlways() & {
        addImpl(static_cast<int>(http::status::internal_server_error), "Internal server error", "ErrorResponse");
        return *this;
    }
    OpenApiResponses&& withAlways() && {
        addImpl(static_cast<int>(http::status::internal_server_error), "Internal server error", "ErrorResponse");
        return std::move(*this);
    }

    // ---- custom
    OpenApiResponses& custom(int status, std::string desc, std::string schemaRef) & {
        addImpl(status, std::move(desc), std::move(schemaRef));
        return *this;
    }
    OpenApiResponses&& custom(int status, std::string desc, std::string schemaRef) && {
        addImpl(status, std::move(desc), std::move(schemaRef));
        return std::move(*this);
    }

    OpenApiResponses& customNoBody(int status, std::string desc) & {
        addNoBodyImpl(status, std::move(desc));
        return *this;
    }
    OpenApiResponses&& customNoBody(int status, std::string desc) && {
        addNoBodyImpl(status, std::move(desc));
        return std::move(*this);
    }

    // ---- finalize
    std::vector<OpenApiResponseMeta> build() && {
        return std::move(items_);
    }
    std::vector<OpenApiResponseMeta> build() const & = delete; // не даём случайно копировать

private:
    std::vector<OpenApiResponseMeta> items_;

    bool hasCode(const int status) const {
        for (const auto& r : items_) if (r.status == status) return true;
        return false;
    }

    void addImpl(const int status, std::string desc, std::optional<std::string> schemaRef) {
        if (!hasCode(status)) {
            items_.push_back({status, std::move(desc), std::move(schemaRef)});
        }
    }

    void addNoBodyImpl(const int status, std::string desc) {
        if (!hasCode(status)) {
            items_.push_back({status, std::move(desc), std::nullopt});
        }
    }

    static std::string refOf(const std::string& s) {
        if (s.rfind("#/", 0) == 0) return s;
        return "#/components/schemas/" + s;
    }
};

#endif //BEAST_API_OPENAPIRESPONSES_H
