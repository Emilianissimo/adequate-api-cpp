#include "core/file_system/magic_mime/MagicMimeDetector.h"

#include <stdexcept>

#include "core/loggers/LoggerSingleton.h"

#if defined(HAVE_LIBMAGIC) && HAVE_LIBMAGIC
    #include <magic.h>

    namespace {
        struct MagicHandle {
            magic_t h{};
            MagicHandle() {
                h = magic_open(MAGIC_MIME_TYPE);
                if (!h) throw std::runtime_error("libmagic: magic_open failed");

                // nullptr => use default magic database search paths
                if (magic_load(h, nullptr) != 0) {
                    std::string msg = magic_error(h) ? magic_error(h) : "libmagic: magic_load failed";
                    magic_close(h);
                    throw std::runtime_error(msg);
                }
            }
            ~MagicHandle() { if (h) magic_close(h); }

            MagicHandle(const MagicHandle&) = delete;
            MagicHandle& operator=(const MagicHandle&) = delete;
        };

        magic_t tls_magic() {
            thread_local MagicHandle mh;
            return mh.h;
        }
    } // namespace
#endif

std::string MagicMimeDetector::detectMime(const std::vector<std::uint8_t>& bytes) const {
    if (bytes.empty()) return {};

#if defined(HAVE_LIBMAGIC) && HAVE_LIBMAGIC
    const magic_t m = tls_magic();
    if (!m) {
        throw std::runtime_error("libmagic: magic_open failed");
    }

    const char* res = magic_buffer(m, bytes.data(), bytes.size());
    if (!res) {
        const char* err = magic_error(m);
        std::string msg = err ? err : "libmagic: magic_buffer failed";
        throw std::runtime_error(msg);
    }

    return {res};
#else
    (void)bytes;
    return {};
#endif
}
