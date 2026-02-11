#pragma once
#include "../core/http/interfaces/HttpInterface.h"
#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"

class HealthController {
public:
    net::awaitable<Outcome> index(Request& request);
};
