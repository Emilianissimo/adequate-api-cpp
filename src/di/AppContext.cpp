#include "di/AppContext.h"
#include <stdexcept>
#include <atomic>
#include "services/users/UsersService.cpp"

namespace {
    std::shared_ptr<AppContext> g_ctx;
    std::atomic<bool> g_inited{false};
}

void appctx::init(std::shared_ptr<AppContext> ctx) {
    if (g_inited.load()) throw std::runtime_error("AppContext already initialized");
    if (!ctx)            throw std::runtime_error("AppContext is null");
    g_ctx = std::move(ctx);
    g_inited.store(true);
}

void appctx::wire(const std::shared_ptr<AppContext>& ctx) {
    ctx->usersRepository = std::make_unique<UsersRepository>(ctx->pg);
    ctx->usersService = std::make_unique<UsersService>(*ctx->usersRepository);
    ctx->usersController = std::make_unique<UsersController>(*ctx->usersService);

    ctx->healthController = std::make_unique<HealthController>();
}

AppContext& appctx::get() {
    if (!g_inited.load() || !g_ctx) throw std::runtime_error("AppContext not initialized");
    return *g_ctx;
}
