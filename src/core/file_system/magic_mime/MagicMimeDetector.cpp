#include <stdexcept>
#include "core/file_system/magic_mime/MagicMimeDetector.h"
#include "core/loggers/LoggerSingleton.h"

/// Working with mutex, if prod will show some latency, we should switch on thread_local magic_t, not static
#if defined(HAVE_LIBMAGIC) && HAVE_LIBMAGIC
magic_t MagicMimeDetector::handle() {
    static magic_t m = [] {
        magic_t h = magic_open(MAGIC_MIME_TYPE);
        if (!h) throw std::runtime_error("libmagic: magic_open failed");

        if (magic_load(h, nullptr) != 0) {
            const char* err = magic_error(h);
            std::string msg = err ? err : "libmagic: magic_load failed";
            magic_close(h);
            throw std::runtime_error(msg);
        }
        return h;
    }();
    return m;
}

std::mutex& MagicMimeDetector::handle_mtx() {
    static std::mutex mtx;
    return mtx;
}
#endif

std::string MagicMimeDetector::detectMime(const std::vector<std::uint8_t>& bytes) const {
    if (bytes.empty()) return {};

#if defined(HAVE_LIBMAGIC) && HAVE_LIBMAGIC
    std::lock_guard lock(handle_mtx());
    magic_t m = handle();

    const char* res = magic_buffer(m, bytes.data(), bytes.size());
    if (!res) {
        const char* err = magic_error(m);
        const std::string msg = err ? err : "libmagic: magic_buffer failed";
        throw std::runtime_error(msg);
    }

    return {res};
#else
    (void)bytes;
    return {};
#endif
}
