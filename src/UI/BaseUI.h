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

    template <typename T, typename DrawFunc, typename RemoveFunc>
    bool DrawCollection(const char* label, T* items, const size_t count, DrawFunc drawFunc, RemoveFunc removeFunc) {
        bool changed = false;

        for (uint32_t i = 0; i < count;) {
            bool remove = false;
            ImGui::PushID(i);
            if (ImGui::SmallButton(std::format("x##{}", label).c_str())) remove = true;
            ImGui::SameLine();
            ImGui::PopID();

            if (ImGui::TreeNode(std::format("{}[{}]", label, i).c_str())) {
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

        return changed;
    }

    template <typename T, typename DrawFunc, typename RemoveFunc>
    bool DrawCollection(const char* label, std::vector<T>& items, DrawFunc drawFunc, RemoveFunc removeFunc) {
        bool changed = false;

        for (uint32_t i = 0; i < items.size();) {
            bool remove = false;
            ImGui::PushID(i);
            if (ImGui::SmallButton(std::format("x##{}", label).c_str())) remove = true;
            ImGui::SameLine();
            ImGui::PopID();

            if (ImGui::TreeNode(std::format("{}[{}]", label, i).c_str())) {
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

        return changed;
    }

    template <typename FullFunc, typename AddFunc>
    bool DrawCollectionHeader(const char* label, const size_t count, FullFunc fullFunc, AddFunc addFunc) {
        bool changed = false;

        ImGui::Text(std::format("{}[{}]", label, count).c_str());

        if (!fullFunc()) {
            ImGui::SameLine();
            if (ImGui::SmallButton(std::format("+##{}", label).c_str())) {
                addFunc();
                changed = true;
            }
        }

        return changed;
    };
}
