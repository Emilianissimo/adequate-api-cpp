#ifndef BEAST_API_TOKENRESPONSESERIALIZER_H
#define BEAST_API_TOKENRESPONSESERIALIZER_H
#include "core/serializers/BaseSerializer.h"
#include "entities/TokenPairEntity.h"
#include "core/loggers/LoggerSingleton.h"

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

#endif //BEAST_API_TOKENRESPONSESERIALIZER_H
