#include "RaytracerUI.h"

#include <string>

#include <imgui.h>

#include "BaseUI.h"
#include "CameraUI.h"
#include "SceneUI.h"

bool UI::DrawRaytracer(Raytracer& raytracer) {
    bool changed = false;

    if (ImGui::Button("Load")) ImGui::OpenPopup("LoadPopup");
    ImGui::SameLine();
    if (ImGui::Button("Save")) ImGui::OpenPopup("SavePopup");

    static std::string filename;

    InputFilenamePopup("LoadPopup",
                       "Load",
                       filename,
                       ".json",
                       [&](const std::filesystem::path& filepath) {
                           changed = raytracer.LoadFromFile(filepath);
                       });

    InputFilenamePopup("SavePopup",
                       "Save",
                       filename,
                       ".json",
                       [&](const std::filesystem::path& filepath) {
                           raytracer.SaveToFile(filepath);
                       });


    if (DrawCamera(raytracer.GetCamera())) {
        raytracer.SetDirty(DirtyFlags::Camera);
    }

    if (DrawScene(raytracer.GetScene())) {
        raytracer.SetDirty(DirtyFlags::Scene);
    }

    return changed;
}
