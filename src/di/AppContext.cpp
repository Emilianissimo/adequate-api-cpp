#include "di/AppContext.h"
#include <stdexcept>
#include <atomic>

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
    FileSystemService::Options fileSystemOptions = {
        ctx->config.root_path,
        ctx->config.file_upload_limit_size,
    };
    ctx->fileSystemService = std::make_unique<FileSystemService>(
        fileSystemOptions
    );
    ctx->jwtService = std::make_unique<JwtService>(ctx->config);
    ctx->healthController = std::make_unique<HealthController>();

    ctx->usersRepository = std::make_unique<UsersRepository>(ctx->pg);
    ctx->usersService = std::make_unique<UsersService>(
        *ctx->usersRepository,
        *ctx->fileSystemService,
        *ctx->blockingPool
    );
    ctx->usersController = std::make_unique<UsersController>(*ctx->usersService);

    // Auth Middleware
    ctx->authenticationMiddleware = std::make_shared<AuthenticationMiddleware>(
        *ctx->jwtService,
        *ctx->usersService
    );

    ctx->authenticationService = std::make_unique<AuthenticationService>(
        *ctx->usersRepository,
        *ctx->jwtService,
        *ctx->blockingPool
    );
    ctx->authenticationController = std::make_unique<AuthenticationController>(
        *ctx->authenticationService,
        *ctx->usersService
    );
}

AppContext& appctx::get() {
    if (!g_inited.load() || !g_ctx) throw std::runtime_error("AppContext not initialized");
    return *g_ctx;
}
