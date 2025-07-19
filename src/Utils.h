#pragma once

#include <expected>
#include <filesystem>
#include <vector>
#include <format>

namespace Utils {
    struct FileError {
        enum class Type {
            NotFound,
            NotAFile,
            OpenFailed,
            ReadFailed
        };

        Type type;
        std::filesystem::path path;
    };

    using FileContent = std::vector<std::byte>;

    std::expected<FileContent, FileError> ReadBinaryFile(const std::filesystem::path& path);
}

template <>
struct std::formatter<Utils::FileError> : std::formatter<std::string> {
    auto format(const Utils::FileError& err, std::format_context& ctx) const {
        std::string_view errorTypeString;
        switch (err.type) {
        case Utils::FileError::Type::NotFound: errorTypeString = "File not found";
            break;
        case Utils::FileError::Type::NotAFile: errorTypeString = "Path is not a regular file";
            break;
        case Utils::FileError::Type::OpenFailed: errorTypeString = "Failed to open file";
            break;
        case Utils::FileError::Type::ReadFailed: errorTypeString = "Failed to read file";
            break;
        default: errorTypeString = "Unknown error";
            break;
        }

        return std::formatter<std::string>::format(std::format("{}: {}", errorTypeString, err.path.string()), ctx);
    }
};
