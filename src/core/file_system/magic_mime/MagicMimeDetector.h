//
// Created by user on 11.02.2026.
//

#ifndef BEAST_API_MAGICMIMEDETECTOR_H
#define BEAST_API_MAGICMIMEDETECTOR_H

#include <cstdint>
#include <string>
#include <mutex>
#include <vector>

#if defined(HAVE_LIBMAGIC) && HAVE_LIBMAGIC
    #include <magic.h>
#endif

class MagicMimeDetector final
{
public:
    [[nodiscard]] std::string detectMime(const std::vector<std::uint8_t>& bytes) const;
private:
#if defined(HAVE_LIBMAGIC) && HAVE_LIBMAGIC
    static magic_t handle();      // init once
    static std::mutex& handle_mtx();
#endif
};


#endif //BEAST_API_MAGICMIMEDETECTOR_H
