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

namespace appctx {
    void init(std::shared_ptr<AppContext> ctx);
    void wire(std::shared_ptr<AppContext> ctx);

    AppContext& get();
};
