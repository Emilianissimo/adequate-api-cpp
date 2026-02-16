#include "core/file_system/magic_mime/MagicMimeDetector.h"

#include <stdexcept>

#include "core/loggers/LoggerSingleton.h"

#if defined(HAVE_LIBMAGIC) && HAVE_LIBMAGIC
    #include <magic.h>
#endif

std::string MagicMimeDetector::detectMime(const std::vector<std::uint8_t>& bytes) const {
    LoggerSingleton::get().info("MagicMimeDetector::detectMime: called", {
        {"bytes_size", std::to_string(bytes.size())},
    });

    if (bytes.empty()) return {};

#if defined(HAVE_LIBMAGIC) && HAVE_LIBMAGIC
    magic_t m = magic_open(MAGIC_MIME_TYPE);
    if (!m) {
        throw std::runtime_error("libmagic: magic_open failed");
    }

    // nullptr => use default magic database search paths
    if (magic_load(m, nullptr) != 0) {
        const char* err = magic_error(m);
        std::string msg = err ? err : "libmagic: magic_load failed";
        magic_close(m);
        throw std::runtime_error(msg);
    }

    const char* res = magic_buffer(m, bytes.data(), bytes.size());
    if (!res) {
        const char* err = magic_error(m);
        std::string msg = err ? err : "libmagic: magic_buffer failed";
        magic_close(m);
        throw std::runtime_error(msg);
    }

    std::string mime(res);
    magic_close(m);
    return mime;
#else
    (void)bytes;
    return {};
#endif
}
