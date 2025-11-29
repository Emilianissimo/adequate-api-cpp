//
// Created by Emil Erofeevskiy on 29/09/25.
//

#include <jwt-cpp/jwt.h>
#include "JwtService.h"

#include "core/loggers/LoggerSingleton.h"

net::awaitable<std::string> JwtService::encode(UserEntity &user) const {
    // I bet we may set those strings as params from env.
    co_return jwt::create()
    .set_issuer("adequate-api")
    .set_type("JWS")
    .set_payload_claim("sub", jwt::claim(std::to_string(user.id)))
    .set_issued_at(std::chrono::system_clock::now())
    .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(1))
    .sign(jwt::algorithm::hs256{this->config_.secret_key});
}

net::awaitable<UserEntity> JwtService::decode(std::string& token) const {
    try {
        const jwt::decoded_jwt data = jwt::decode(token);
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{this->config_.secret_key})
            .with_issuer("adequate-api")
            .verify(data);
        UserEntity user;
        user.id = data.get_payload_claim("sub").as_integer();
        co_return user;
    }
    catch (const jwt::error::signature_verification_exception& e) {
        LoggerSingleton::get().error(
            "JwtService::decode: Decoding error: " + std::string(e.what())
        );
        throw JwtException(JwtError::InvalidSignature, e.what());
    }
    catch (const jwt::error::token_verification_exception& e) {
        LoggerSingleton::get().error(
            "JwtService::decode: Decoding error: " + std::string(e.what())
        );
        throw JwtException(JwtError::TokenExpired, e.what());
    }
    catch (const jwt::error::claim_not_present_exception& e) {
        LoggerSingleton::get().error(
            "JwtService::decode: Decoding error: " + std::string(e.what())
        );
        throw JwtException(JwtError::InvalidClaim, e.what());
    }
    catch (const std::exception& e) {
        LoggerSingleton::get().error(
            "JwtService::decode: Decoding error: " + std::string(e.what())
        );
        throw JwtException(JwtError::DecodeError, e.what());
    }
    catch (...) {
        LoggerSingleton::get().error("JwtService::decode: Decoding error: Unknown JWT error");
        throw JwtException(JwtError::Unknown, "Unknown JWT error");
    }
}
