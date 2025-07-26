#include "Utils.h"

#include <fstream>
#include <cstring> // memcpy

namespace Utils {
    std::expected<std::vector<std::byte>, FileError> ReadBinaryFile(const std::filesystem::path& path) {
        namespace fs = std::filesystem;

        if (!fs::exists(path)) {
            return std::unexpected(FileError{FileError::Type::NotFound, path});
        }

        if (!fs::is_regular_file(path)) {
            return std::unexpected(FileError{FileError::Type::NotAFile, path});
        }

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            return std::unexpected(FileError{FileError::Type::OpenFailed, path});
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<std::byte> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            return std::unexpected(FileError{FileError::Type::ReadFailed, path});
        }

        return buffer;
    }

    std::expected<std::vector<uint32_t>, FileError> ReadSpirvFile(const std::filesystem::path& path) {
        auto raw = ReadBinaryFile(path);
        if (!raw) return std::unexpected(raw.error());

        if (raw->size() % sizeof(uint32_t) != 0) {
            return std::unexpected(FileError{FileError::Type::ReadFailed, path});
        }

        const size_t wordCount = raw->size() / sizeof(uint32_t);
        std::vector<uint32_t> result(wordCount);
        memcpy(result.data(), raw->data(), raw->size());
        return result;
    }
}
