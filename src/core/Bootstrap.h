#pragma once
#include "core/configs/EnvConfig.h"
#include "core/routers/Router.h"

class Bootstrap {
public:
    static int run(net::io_context& ioc, const EnvConfig& env, Router& router);
};
