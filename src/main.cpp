#include "core/Bootstrap.h"
#include "core/configs/EnvConfig.h"
#include "core/routers/Router.h"
#include "routes/Routes.h"
#include "core/db/postgres/interfaces/PgPool.h"
#include "di/AppContext.h"

int main() {
    auto env = EnvConfig::load();

    boost::asio::io_context ioc{
        static_cast<int>(std::max(1u, std::thread::hardware_concurrency()))
    };

    // DI context
    AppContext ctx;
    ctx.pg = std::make_shared<PgPool>(ioc.get_executor(), env.pg_dsn, env.pg_pool_size);
    // plug any other
    // --- end DI context

    Router router;
    app::define_routes(router, ctx);
    Bootstrap bootstrap;
    return bootstrap.run(ioc, env, router);
}
