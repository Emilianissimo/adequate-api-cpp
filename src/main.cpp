#include "core/Bootstrap.h"
#include "core/configs/EnvConfig.h"
#include "core/routers/Router.h"
#include "routes/Routes.h"

int main() {
    auto env = EnvConfig::load();
    Router router;
    app::define_routes(router);
    Bootstrap bootstrap;
    return bootstrap.run(env, router);
}
