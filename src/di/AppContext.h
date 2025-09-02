#pragma once
#include <memory>
#include "core/db/postgres/interfaces/PgPool.h"

#include "controllers/HealthController.h"
// Users
#include "repositories/users/UsersRepository.h"
#include "services/users/UsersService.cpp"
#include "controllers/UsersController.h"

struct AppContext {
    std::shared_ptr<PgPool> pg;
    // std::shared_ptr<Redis>  redis;

    std::unique_ptr<HealthController> healthController;

    std::unique_ptr<UsersRepository> usersRepository;
    std::unique_ptr<UsersServiceInterface> usersService;
    std::unique_ptr<UsersController> usersController;
};

void wire(AppContext& ctx) {
    ctx.usersRepository = std::make_unique<UsersRepository>(ctx.pg);
    ctx.usersService = std::make_unique<UsersService>(*ctx.usersRepository);
    ctx.usersController = std::make_unique<UsersController>(*ctx.usersService);

    ctx.healthController = std::make_unique<HealthController>();
}

namespace appctx {
    void init(std::shared_ptr<AppContext> ctx);

    AppContext& get();
};
