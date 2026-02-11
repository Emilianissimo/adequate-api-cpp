#pragma once
#include <cstdint>
#include <string>

struct EnvConfig {
    std::string host = "0.0.0.0";
    uint16_t port = 8080;
    std::string pg_dsn;
    std::size_t pg_pool_size = 10;
    std::string redis_host;
    uint16_t redis_port = 6379;
    std::string redis_password;
    std::string secret_key;
    uint64_t file_upload_limit_size;
    std::string multipart_adapter;

    static EnvConfig load();
};
