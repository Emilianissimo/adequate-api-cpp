//
// Created by user on 10.02.2026.
//

#include "FileSystemService.h"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <fstream>
#include <random>
#include <regex>
#include <system_error>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>

#include "magic_mime.hpp"

#include "errors/FileSystemErrors.h"

namespace app
{
    static std::string makeErr(const std::string& what, const std::filesystem::path& path)
    {
        return what + "+ " + path.string();
    }

    static const app::MagicMimeDetector& mimeDetector() {
        static app::MagicMimeDetector det;
        return det;
    }

    FileSystemService::FileSystemService(Options options) : opts_(std::move(options))
    {
        if (opts_.rootPath.empty())
        {
            throw FileValidationError("FilesystemService: rootDir is empty");
        }

        std::error_code ec;
        std::filesystem::create_directories(opts_.rootPath, ec);
        if (ec)
        {
            throw FileIOError("FilesystemService: cannot create rootPath: " + opts_.rootPath.string() +
                          " (" + ec.message() + ")");
        }
    }

    StoredFileInfo FileSystemService::store(
        std::string_view entityName,
        std::string_view entityId,
        const IncomingFile& file) const
    {
        const std::string normEntity = normalizeEntityName(entityName);
        const std::string normId = normalizeEntityId(entityId);

        validateFile(opts_, file);

        const std::filesystem::path relativePath = buildRelativeDir(normEntity, normId);
        const std::filesystem::path fullPath = opts_.rootPath / relativePath;

        std::error_code ec;
        std::filesystem::create_directories(fullPath, ec);
        if (ec)
        {
            throw FileIOError("Failed to create directories: " + fullPath.string() + " (" + ec.message() + ")");
        }

        std::string ext;
        if (opts_.preserveAllowedExtension && !opts_.allowedExtensions.empty())
        {
            ext = pickAllowedExtension(opts_, file.originalFileName);
        }

        std::string name = randomHex(16);
        if (!ext.empty())
        {
            name += ".";
            name += ext;
        }

        const std::filesystem::path finalPath = fullPath / name;

        ensureWithinRoot(opts_.rootPath, finalPath);

        std::uintmax_t bytesWritten = 0;
        writeFileAtomic(finalPath, file.bytes, bytesWritten);

        StoredFileInfo info;
        info.relativePath = relativePath / name;
        info.storedFileName = name;
        info.bytesWritten = bytesWritten;
        return info;
    }

    boost::asio::awaitable<StoredFileInfo> FileSystemService::storeAsync(
        boost::asio::any_io_executor poolExec,
        std::string entityName,
        std::string entityId,
        IncomingFile file
    ) const
    {
        // Important: Offload blocking work to a separate executor (thread pool).
        // exec must be a pool executor, not an io_context with a single thread.

        co_return co_await boost::asio::co_spawn(
            poolExec,
            [this,
             entityName = std::move(entityName),
             entityId   = std::move(entityId),
             file       = std::move(file)]() mutable -> boost::asio::awaitable<StoredFileInfo>
            {
                co_return this->store(entityName, entityId, file);
            },
            boost::asio::use_awaitable
        );
    }

    void FileSystemService::remove(std::filesystem::path relativePath) const
    {
        if (relativePath.empty()) return;

        const std::filesystem::path fullPath = opts_.rootPath / relativePath;
        ensureWithinRoot(opts_.rootPath, fullPath);

        std::error_code ec;
        std::filesystem::remove(fullPath, ec);
        if (ec)
        {
            throw FileIOError("Failed to remove file: " + fullPath.string() + " (" + ec.message() + ")");
        }
    }

    bool FileSystemService::exists(std::filesystem::path relativePath) const
    {
        if (relativePath.empty()) return false;
        const std::filesystem::path fullPath = opts_.rootPath / relativePath;
        ensureWithinRoot(opts_.rootPath, fullPath);

        std::error_code ec;
        const bool ok = std::filesystem::exists(fullPath, ec);
        return !ec && ok;
    }

    std::string FileSystemService::toLower(std::string input)
    {
        std::ranges::transform(input, input.begin(),
                               [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return input;
    }

    bool FileSystemService::isValidEntityName(const std::string_view input)
    {
        static const std::regex re("^[a-z0-9_]+$");
        return std::regex_match(std::string(input), re);
    }

    bool FileSystemService::isValidEntityId(std::string_view input) const
    {
        if (input.empty()) return false;

        if (!opts_.allowNonNumericEntityId) {
            return std::all_of(input.begin(), input.end(), [](unsigned char c){ return std::isdigit(c); });
        }

        // allow [a-z0-9_-]+
        static const std::regex re("^[a-z0-9_-]+$");
        return std::regex_match(std::string(input), re);
    }

    std::string FileSystemService::normalizeEntityName(std::string_view input) {
        std::string v(input);
        v = toLower(std::move(v));
        if (!isValidEntityName(v)) {
            throw FileSecurityError("Invalid entity name (allowed: [a-z0-9_]+)");
        }
        return v;
    }

    std::string FileSystemService::normalizeEntityId(std::string_view input) const {
        std::string v(input);
        v = toLower(std::move(v));
        if (!isValidEntityId(v)) {
            throw FileSecurityError("Invalid entity id");
        }
        return v;
    }

    std::filesystem::path FileSystemService::buildRelativeDir(const std::string_view entityName,
                                                         const std::string_view entityId)
    {
        // entity/id/file
        return std::filesystem::path(entityName) / std::filesystem::path(entityId) / "file";
    }

    std::string FileSystemService::pickAllowedExtension(const Options& opts,
                                                    const std::string_view originalFileName)
    {
        if (originalFileName.empty()) return {};

        const std::filesystem::path p{std::string(originalFileName)};
        std::string ext = p.extension().string(); // includes dot
        if (ext.empty()) return {};

        // strip dot
        if (!ext.empty() && ext.front() == '.') ext.erase(ext.begin());
        ext = toLower(std::move(ext));

        if (!opts.allowedExtensions.contains(ext)) {
            // not allowed => store without extension
            return {};
        }
        return ext;
    }

    std::string FileSystemService::randomHex(const std::size_t bytesLen) {
        thread_local std::mt19937_64 rng(std::random_device{}());
        static constexpr char hex[] = "0123456789abcdef";

        std::string out;
        out.resize(bytesLen * 2);

        for (std::size_t i = 0; i < bytesLen; ++i) {
            const std::uint8_t b = static_cast<std::uint8_t>(rng() & 0xFFu);
            out[i * 2 + 0] = hex[(b >> 4) & 0x0F];
            out[i * 2 + 1] = hex[(b >> 0) & 0x0F];
        }
        return out;
    }

    void FileSystemService::ensureWithinRoot(const std::filesystem::path& root,
                                        const std::filesystem::path& fullPath)
    {
        // Weakly canonical to tolerate non-existent final file.
        std::error_code ec1, ec2;
        const auto canonRoot = std::filesystem::weakly_canonical(root, ec1);
        const auto canonFull = std::filesystem::weakly_canonical(fullPath, ec2);

        // If canonicalization fails, still try lexical fallback (but be strict).
        const auto& r = ec1 ? root.lexically_normal() : canonRoot;
        const auto& f = ec2 ? fullPath.lexically_normal() : canonFull;

        // Check prefix: root must be a leading path of full
        auto rit = r.begin();
        auto fit = f.begin();
        for (; rit != r.end(); ++rit, ++fit) {
            if (fit == f.end() || *rit != *fit) {
                throw FileSecurityError("Path escapes rootDir: " + fullPath.string());
            }
        }
        // root fully matched => ok
    }

    void FileSystemService::validateFile(const Options& opts, const IncomingFile& file) {
        if (file.bytes.empty()) {
            throw FileValidationError("File is empty");
        }

        if (const std::uintmax_t sz = file.bytes.size(); opts.maxBytes > 0 && sz > opts.maxBytes) {
            throw FileValidationError("File too large: " + std::to_string(sz) +
                                      " > " + std::to_string(opts.maxBytes));
        }

        // --- MIME check (lightweight, NOT deep validation) ---
        if (!opts.allowedMime.empty()) {
            std::string mime;
            try {
                mime = mimeDetector().detectMime(file.bytes);
            } catch (const std::exception& e) {
                if (opts.strictMime) {
                    throw FileValidationError(std::string("MIME detection failed: ") + e.what());
                }
                return; // non-strict => allow
            }

            if (mime.empty()) {
                if (opts.strictMime) throw FileValidationError("Unable to detect MIME type");
                return;
            }

            if (!opts.allowedMime.contains(mime)) {
                throw FileValidationError("Unsupported MIME type: " + mime);
            }
        }

        // --- Optional extension strictness (still not trusted, but can be an extra gate) ---
        if (!opts.allowedExtensions.empty() && opts.preserveAllowedExtension) {
            // If you want strict ext: uncomment.
            // auto ext = pickAllowedExtension(opts, file.originalFileName);
            // if (ext.empty()) throw FileValidationError("Unsupported file extension");
        }
    }

    void FileSystemService::writeFileAtomic(const std::filesystem::path& finalPath,
                                       const std::vector<std::uint8_t>& bytes,
                                       std::uintmax_t& bytesWritten)
    {
        const auto dir = finalPath.parent_path();

        // temp name in same directory (so rename is atomic on same filesystem)
        const std::string tmpName = ".tmp_" + randomHex(8);
        const std::filesystem::path tmpPath = dir / tmpName;

        // Write temp
        {
            std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
            if (!out.is_open()) {
                throw FileIOError(makeErr("Cannot open temp file for write", tmpPath));
            }
            if (!bytes.empty()) {
                out.write(reinterpret_cast<const char*>(bytes.data()),
                          static_cast<std::streamsize>(bytes.size()));
                if (!out.good()) {
                    out.close();
                    std::error_code ec;
                    std::filesystem::remove(tmpPath, ec);
                    throw FileIOError(makeErr("Write failed", tmpPath));
                }
            }
            out.flush();
            if (!out.good()) {
                out.close();
                std::error_code ec;
                std::filesystem::remove(tmpPath, ec);
                throw FileIOError(makeErr("Flush failed", tmpPath));
            }
        }

        bytesWritten = bytes.size();

        // Rename temp -> final
        std::error_code ec;
        std::filesystem::rename(tmpPath, finalPath, ec);
        if (ec) {
            // cleanup tmp
            std::error_code ec2;
            std::filesystem::remove(tmpPath, ec2);
            throw FileIOError("Rename failed: " + tmpPath.string() + " -> " + finalPath.string() +
                              " (" + ec.message() + ")");
        }
    }
}
