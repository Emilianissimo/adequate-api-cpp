#pragma once

#include "core/db/postgres/interfaces/PgPool.h"
#include <memory>

class BaseRepository {
public:
    explicit BaseRepository(std::shared_ptr<PgPool> pool) : pool_(std::move(pool)) {}

protected:
    std::shared_ptr<PgPool> pool_;
};
