//
// Created by user on 10.02.2026.
//

#ifndef BEAST_API_FILESYSTEMSERVICE_H
#define BEAST_API_FILESYSTEMSERVICE_H

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/any_io_executor.hpp>

namespace app
{
    struct StoredFileInfo
{
    std::filesystem::path relativePath; // entity/id/file_name
    std::string storedFileName;
    std::uintmax_t bytesWritten = 0;
};

struct IncomingFile
{
    std::vector<std::uint8_t> bytes;
    std::string originalFileName; // user-provided filename
    std::string contentType;
};

class FileSystemService
{
public:
    struct Options
    {
        std::filesystem::path rootPath;
        // Default 2GB of filesize, adjusts on env
        std::uintmax_t maxBytes = 10u * 1024u * 1024u;
        std::unordered_set<std::string> allowedExtensions = {
            "png",
            "jpg",
            "jpeg",
            "webp",
            "gif",
            "pdf",
            "txt"
        };

        // If true, we try to preserve extension from original filename,
        // but only if it is in allowedExtensions. Otherwise, we store without extension.
        // MIME allowlist (detected from bytes). Leave empty to disable MIME checks.
        std::unordered_set<std::string> allowedMime = {
            "image/png", "image/jpeg", "image/webp", "application/pdf", "text/plain"
        };

        // If true -> reject if MIME can't be detected or not in allowlist
        bool strictMime = true;

        bool preserveAllowedExtension = true;
        bool allowNonNumericEntityId = false;
    };

    explicit FileSystemService(Options options);

    StoredFileInfo store(std::string_view entityName, std::string_view entityId, const IncomingFile& file) const;
    boost::asio::awaitable<StoredFileInfo> storeAsync(
        boost::asio::any_io_executor poolExec,
        std::string entityName,
        std::string entityId,
        IncomingFile file
    ) const;

    void remove(std::filesystem::path relativePath) const;

    bool exists(std::filesystem::path relativePath) const;

    [[nodiscard]] const Options& options() const noexcept { return opts_; };

private:
    Options opts_;

    static std::string normalizeEntityName(std::string_view input);
    std::string normalizeEntityId(std::string_view input) const;

    static std::string toLower(std::string input);
    static bool isValidEntityName(std::string_view input);
    bool isValidEntityId(std::string_view input) const;

    static std::filesystem::path buildRelativeDir(std::string_view entityName, std::string_view entityId);
    static std::string pickAllowedExtension(const Options& opts, std::string_view originalFileName);

    static std::string randomHex(std::size_t bytesLen);

    static void ensureWithinRoot(const std::filesystem::path& root, const std::filesystem::path& fullPath);

    static void writeFileAtomic(
        const std::filesystem::path& finalPath,
        const std::vector<std::uint8_t>& bytes,
        std::uintmax_t& bytesWritten
    );

    static void validateFile(const Options& opts, const IncomingFile& file);
};

}

#endif //BEAST_API_FILESYSTEMSERVICE_H