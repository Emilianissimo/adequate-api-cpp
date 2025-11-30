//
// Created by Emil Erofeevskiy on 29/09/25.
//
#pragma once
#ifndef BEAST_API_JWTSERVICE_H
#define BEAST_API_JWTSERVICE_H

#include "core/configs/EnvConfig.h"
#include "entities/UserEntity.h"
#include "core/interfaces/HttpInterface.h"
#include "repositories/users/UsersRepository.h"

enum class JwtError {
    InvalidSignature,
    TokenExpired,
    InvalidClaim,
    DecodeError,
    Unknown
};

class JwtException final : public std::exception {
public:
    JwtException(JwtError err, std::string msg)
        : err_(err), msg_(std::move(msg)) {}

    [[nodiscard]] const char* what() const noexcept override {
        return msg_.c_str();
    }

    [[nodiscard]] JwtError code() const noexcept { return err_; }

private:
    JwtError err_;
    std::string msg_;
};

class JwtService {
public:
    explicit JwtService(EnvConfig& config) : config_(config) {};
    net::awaitable<std::string> encode(UserEntity& user) const;
    net::awaitable<UserEntity> decode(std::string& token) const;
private:
    EnvConfig& config_;
};


#endif //BEAST_API_JWTSERVICE_H