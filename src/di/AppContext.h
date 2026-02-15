#pragma once
#include <memory>
#include "core/db/postgres/interfaces/PgPool.h"
#include "core/configs/EnvConfig.h"
#include "services/jwt/JwtService.h"

/// Health
#include "controllers/HealthController.h"
/// Users
#include "controllers/UsersController.h"
#include "repositories/users/UsersRepository.h"
#include "services/users/UsersService.h"
/// Authentication
#include "controllers/auth/AuthenticationController.h"
#include "middlewares/AuthenticationMiddleware.h"
#include "services/auth/AuthenticationService.h"

struct AppContext {
    std::shared_ptr<PgPool> pg;
    // std::shared_ptr<Redis>  redis;

    EnvConfig config;
    std::unique_ptr<JwtService> jwtService;
    std::unique_ptr<FileSystemService> fileSystemService;

    std::unique_ptr<HealthController> healthController;

    std::unique_ptr<UsersRepository> usersRepository;
    std::unique_ptr<UsersService> usersService;
    std::unique_ptr<UsersController> usersController;

    std::unique_ptr<AuthenticationService> authenticationService;
    std::unique_ptr<AuthenticationController> authenticationController;

    std::shared_ptr<AuthenticationMiddleware> authenticationMiddleware;
};

namespace appctx {
    void init(std::shared_ptr<AppContext> ctx);
    void wire(const std::shared_ptr<AppContext>& ctx);

    AppContext& get();
};
