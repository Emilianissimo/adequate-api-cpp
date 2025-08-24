#pragma once
#include "core/configs/EnvConfig.h"
#include "core/routers/Router.h"

class Bootstrap {
public:
    int run(const EnvConfig& env, Router& router);
};
