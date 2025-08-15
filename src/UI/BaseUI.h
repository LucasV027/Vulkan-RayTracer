#pragma once

#include <string>
#include <filesystem>
#include <functional>

namespace UI {
    void LoadPopup(std::string& filename,
                   const std::filesystem::path& extension,
                   const std::function<void(const std::filesystem::path&)>& callback);
}
