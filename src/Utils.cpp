#include "Utils.h"

#include <fstream>

namespace Utils {
    std::expected<FileContent, FileError> ReadBinaryFile(const std::filesystem::path& path) {
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
}
