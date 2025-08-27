#pragma once
#include <memory>
#include "core/db/postgres/interfaces/PgPool.h"

struct AppContext {
    std::shared_ptr<PgPool> pg;
    // std::shared_ptr<Redis>  redis;
};

namespace appctx {
    void init(std::shared_ptr<AppContext> ctx);

    AppContext& get();
};
