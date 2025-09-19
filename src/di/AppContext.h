#pragma once
#include <memory>
#include "core/db/postgres/interfaces/PgPool.h"

/// Health
#include "controllers/HealthController.h"
/// Users
#include "controllers/UsersController.h"
#include "repositories/users/UsersRepository.h"
#include "services/users/UsersService.h"
/// Authentication
#include "controllers/auth/AuthenticationController.h"
#include "services/auth/AuthenticationService.h"

struct AppContext {
    std::shared_ptr<PgPool> pg;
    // std::shared_ptr<Redis>  redis;

    std::unique_ptr<HealthController> healthController;

    std::unique_ptr<UsersRepository> usersRepository;
    std::unique_ptr<UsersService> usersService;
    std::unique_ptr<UsersController> usersController;

    std::unique_ptr<AuthenticationService> authenticationService;
    std::unique_ptr<AuthenticationController> authenticationController;
};

namespace appctx {
    void init(std::shared_ptr<AppContext> ctx);
    void wire(const std::shared_ptr<AppContext>& ctx);

    AppContext& get();
};
