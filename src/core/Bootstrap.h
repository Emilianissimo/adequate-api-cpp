#pragma once
#include "core/configs/EnvConfig.h"
#include "core/routers/Router.h"

class Bootstrap {
public:
    int run(boost::asio::io_context& ioc, const EnvConfig& env, Router& router);
};
