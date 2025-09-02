#include "core/db/postgres/interfaces/PgPool.h"
#include <memory>

class BaseRepository {
public:
    BaseRepository(std::shared_ptr<PgPool> pool) : pool_(std::move(pool)) {}
private:
    std::shared_ptr<PgPool> pool_;
};
