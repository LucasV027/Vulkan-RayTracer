#include "BaseUI.h"

#include <filesystem>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void UI::LoadPopup(std::string& filename,
                   const std::filesystem::path& extension,
                   const std::function<void(const std::filesystem::path&)>& callback) {
    static auto inputTextCallback = [](ImGuiInputTextCallbackData* data) {
        if (isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == '-') return 0;
        return 1;
    };

    ImGui::Text("Enter filename:");
    ImGui::InputText("##Filename", &filename, ImGuiInputTextFlags_CallbackCharFilter, inputTextCallback);

    ImGui::Separator();

    if (ImGui::Button("Load")) {
        if (!filename.empty()) {
            auto filePath = ASSETS_PATH / static_cast<std::filesystem::path>(filename);
            filePath.replace_extension(".obj");
            callback(filePath);
            filename.clear();
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
        filename.clear();
        ImGui::CloseCurrentPopup();
    }
}
