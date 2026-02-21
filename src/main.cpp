#include <sodium.h>
#include "core/Bootstrap.h"
#include "core/configs/EnvConfig.h"
#include "core/routers/Router.h"
#include "routes/Routes.h"
#include "core/db/postgres/interfaces/PgPool.h"
#include "core/hashers/SodiumPasswordHasher.h"
#include "di/AppContext.h"
#include "core/loggers/LoggerFactory.h"
#include "core/loggers/LoggerSingleton.h"

inline void init_crypto_once() {
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium: sodium_init failed");
    }
}

int main() {
    EnvConfig env = EnvConfig::load();
    init_crypto_once();

    // Singletons
    LoggerSingleton::init(
        LoggerFactory::create("console")
    );

    /// Using 1 I/O, so I would be sure, that blocking tasks will run correctly
    /// Only because of them
    // net::io_context ioc{1};
    /// In case of using net::stand we can use
    net::io_context ioc{
        static_cast<int>(std::max(1u, std::thread::hardware_concurrency()))
    };

    // Create a pool.
    // Note: thread_pool starts immediately upon creation.
    auto blockingPool = std::make_shared<net::thread_pool>(
        std::thread::hardware_concurrency()
    );
    app::security::SodiumPasswordHasher passwordHasher{
        app::security::SodiumPasswordHasher::defaultForBuild()
    };

    // DI context
    const auto ctx = std::make_shared<AppContext>();
    ctx->pg = std::make_shared<PgPool>(ioc.get_executor(), env.pg_dsn, env.pg_pool_size);
    ctx->blockingPool = blockingPool;
    ctx->config = env;
    ctx->passwordHasher = std::make_shared<app::security::SodiumPasswordHasher>(passwordHasher);
    appctx::init(ctx);
    appctx::wire(ctx);
    // plug any other
    // --- end DI context

    Router router;
    app::define_routes(router, ctx);
    Bootstrap bootstrap;
    int result =  bootstrap.run(ioc, env, router);

    // Important: join on exit, to ensure graceful end of tasks
    blockingPool->join();
    return result;
}
