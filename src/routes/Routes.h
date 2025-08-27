#pragma once
#include "core/routers/Router.h"
#include "di/AppContext.h"

namespace app {
    void define_routes(Router& router, const AppContext& ctx);
}
