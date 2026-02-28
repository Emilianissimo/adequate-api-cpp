#ifndef BEAST_API_TOKENRESPONSESERIALIZER_H
#define BEAST_API_TOKENRESPONSESERIALIZER_H

#include "core/serializers/BaseSerializer.h"
#include "entities/TokenPairEntity.h"
#include "core/loggers/LoggerSingleton.h"
#include "core/openapi/specs/OpenApiSchemaSpec.h"

class TokenResponseSerializer final : public BaseSerializer<TokenResponseSerializer, TokenPairEntity> {
public:
    TokenResponseSerializer() = default;

    std::string refreshToken;
    std::string accessToken;

    static TokenResponseSerializer fromEntity(const TokenPairEntity& entity) {
        LoggerSingleton::get().info("TokenResponseSerializer::fromEntity: started");
        TokenResponseSerializer serializer;
        serializer.refreshToken = entity.refreshToken;
        serializer.accessToken = entity.accessToken;
        return serializer;
    }

    /// Covering to_json / from_json declarative way
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        TokenResponseSerializer,
        refreshToken,
        accessToken
    )
};

template <>
struct OpenApiSchemaSpec<TokenResponseSerializer> {
    static std::string name() { return "LoginResponse"; }

    static nlohmann::json schema() {
        return {
                {"type","object"},
                {"required", {"refreshToken","accessToken"}},
                {"properties",{
                    {"refreshToken", {{"type","string"}}},
                    {"accessToken", {{"type","string"}}},
                }},
                {"additionalProperties", false}
        };
    }
};

#endif //BEAST_API_TOKENRESPONSESERIALIZER_H
