#include "core/Bootstrap.h"
#include "core/configs/EnvConfig.h"
#include "core/routers/Router.h"
#include "routes/Routes.h"
#include "core/db/postgres/interfaces/PgPool.h"
#include "di/AppContext.h"
#include "core/loggers/LoggerSingleton.h"
#include "core/loggers/strategies/ConsoleLoggerStrategy.h"

int main() {
    auto env = EnvConfig::load();

    // Singletons
    LoggerSingleton::init(std::make_shared<ConsoleLoggerStrategy>());

    boost::asio::io_context ioc{
        static_cast<int>(std::max(1u, std::thread::hardware_concurrency()))
    };

    // DI context
    const auto ctx = std::make_shared<AppContext>();
    ctx->pg = std::make_shared<PgPool>(ioc.get_executor(), env.pg_dsn, env.pg_pool_size);
    appctx::init(ctx);
    appctx::wire(ctx);
    // plug any other
    // --- end DI context

    Router router;
    app::define_routes(router, ctx);
    Bootstrap bootstrap;
    return bootstrap.run(ioc, env, router);
}
