#pragma once

#include <string>
#include <filesystem>
#include <functional>
#include <format>

#include <imgui.h>

namespace UI {
    void InputFilenamePopup(const char* strId,
                            const char* buttonLabel,
                            std::string& filename,
                            const std::filesystem::path& extension,
                            const std::function<void(const std::filesystem::path&)>& callback);

    template <typename T, typename FullFunc, typename AddFunc, typename DrawFunc, typename RemoveFunc>
    bool DrawCollection(const char* label,
                        std::vector<T>& items,
                        FullFunc fullFunc,
                        AddFunc addFunc,
                        DrawFunc drawFunc,
                        RemoveFunc removeFunc) {
        bool changed = false;

        ImGui::PushID("label");

        // Header
        {
            ImGui::Text("%s[%u]", label, items.size());

            if (!fullFunc()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("+")) {
                    addFunc();
                    changed = true;
                }
            }
        }

        ImGui::Separator();

        // Body
        {
            ImGui::Indent();

            for (uint32_t i = 0; i < items.size();) {
                bool remove = false;
                ImGui::PushID(i);
                if (ImGui::SmallButton("x")) remove = true;
                ImGui::SameLine();
                ImGui::PopID();

                static std::string itemStr;
                itemStr = std::format("{}[{}]", label, i);
                if (ImGui::TreeNode(itemStr.c_str())) {
                    changed |= drawFunc(items[i]);
                    ImGui::TreePop();
                }

                if (remove) {
                    removeFunc(i);
                    changed = true;
                } else {
                    i++;
                }
            }

            ImGui::Unindent();
        }

        ImGui::PopID();
        return changed;
    }
}
