//
// Created by user on 11.02.2026.
//

#ifndef BEAST_API_MAGICMIMEDETECTOR_H
#define BEAST_API_MAGICMIMEDETECTOR_H

#pragma once

#include <cstdint>
#include <string>
#include <vector>

class MagicMimeDetector final
{
public:
    [[nodiscard]] std::string detectMime(const std::vector<std::uint8_t>& bytes) const;
};


#endif //BEAST_API_MAGICMIMEDETECTOR_H
