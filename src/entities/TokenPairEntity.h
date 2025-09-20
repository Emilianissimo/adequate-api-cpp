#pragma once
#include <string>
#include "core/interfaces/EntityInterface.h"

struct TokenPairEntity final : EntityInterface {
    std::string refreshToken;
    std::string accessToken;
};
