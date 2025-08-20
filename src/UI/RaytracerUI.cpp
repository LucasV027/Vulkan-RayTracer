#include "RaytracerUI.h"

#include <string>

#include <imgui.h>

#include "BaseUI.h"
#include "CameraUI.h"
#include "SceneUI.h"

void UI::DrawRaytracer(Raytracer& raytracer) {
    if (ImGui::Button("Load")) ImGui::OpenPopup("LoadPopup");
    ImGui::SameLine();
    if (ImGui::Button("Save")) ImGui::OpenPopup("SavePopup");

    static std::string filename;

    InputFilenamePopup("LoadPopup",
                       "Load",
                       filename,
                       ".json",
                       [&](const std::filesystem::path& filepath) {
                           raytracer.LoadFromFile(filepath);
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
        raytracer.SetDirty(DirtyFlags::BVH);
    }
}
