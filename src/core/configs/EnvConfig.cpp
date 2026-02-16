#include "core/configs/EnvConfig.h"
#include <stdexcept>

static std::string getEnvOrDefault(const char* key, const std::string& default_ = "") 
{
    if (const char* value = std::getenv(key)){
        return value;
    }
    return default_;
};

static uint16_t getEnvOrDefaultUint16(const char* key, const uint16_t default_) 
{
    if (const char* value = std::getenv(key)){
        try {
            return static_cast<uint16_t>(std::stoi(value));
        } catch (...) {
            throw std::runtime_error(std::string("Invalid integer for env variable: ") + key);
        }
    }
    return default_;
};

static uint64_t getEnvOrDefaultUint64(const char* key, const uint32_t default_)
{
    if (const char* value = std::getenv(key)){
        try {
            return static_cast<uint64_t>(std::stoi(value));
        } catch (...) {
            throw std::runtime_error(std::string("Invalid integer for env variable: ") + key);
        }
    }
    return default_;
};

EnvConfig EnvConfig::load()
{
    EnvConfig config;
    config.host                   = getEnvOrDefault("APP_HOST", "0.0.0.0");
    config.port                   = getEnvOrDefaultUint16("APP_PORT", 8080);
    config.pg_dsn                 = getEnvOrDefault("DATABASE_URL");
    config.redis_host             = getEnvOrDefault("REDIS_HOST", "127.0.0.1");
    config.redis_port             = getEnvOrDefaultUint16("REDIS_PORT", 6379);
    config.redis_password         = getEnvOrDefault("REDIS_PASSWORD", "");
    config.secret_key             = getEnvOrDefault("SECRET_KEY", "");
    config.multipart_adapter      = getEnvOrDefault("MULTIPART_ADAPTER", "POCO");
    config.file_upload_limit_size = getEnvOrDefaultUint64("FILE_UPLOAD_LIMIT_SIZE", 0);
    config.media_path              = getEnvOrDefault("MEDIA_PATH", "media");

    return config;
}
