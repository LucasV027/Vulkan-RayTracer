#include "BaseUI.h"

#include <filesystem>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

static auto inputTextCallback = [](ImGuiInputTextCallbackData* data) {
    if (isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == '-') return 0;
    return 1;
};

void UI::InputFilenamePopup(const char* strId,
                            std::string& filename,
                            const std::filesystem::path& extension,
                            const std::function<void(const std::filesystem::path&)>& callback) {
    if (ImGui::BeginPopupModal(strId, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter filename:");
        ImGui::InputText("##Filename", &filename, ImGuiInputTextFlags_CallbackCharFilter, inputTextCallback);

        ImGui::Separator();

        if (ImGui::Button("Load")) {
            if (!filename.empty()) {
                auto filePath = ASSETS_PATH / static_cast<std::filesystem::path>(filename);
                filePath.replace_extension(extension);
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
        ImGui::EndPopup();
    }
}
