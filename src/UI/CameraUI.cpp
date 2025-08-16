#include "CameraUI.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

bool UI::DrawCamera(Camera& camera) {
    bool changed = false;

    static float baseFov = camera.GetFovDeg();
    static glm::vec3 baseCameraPos = camera.GetPosition();
    static glm::vec3 baseCameraForward = camera.GetForward();

    float fovDeg = camera.GetFovDeg();
    glm::vec3 cameraPos = camera.GetPosition();
    glm::vec3 cameraForward = camera.GetForward();

    if (ImGui::TreeNode("Camera")) {
        ImGui::Indent();

        ImGui::SeparatorText("Transform");
        if (ImGui::DragFloat3("Position", glm::value_ptr(cameraPos), 0.1f)) {
            camera.SetPosition(cameraPos);
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##Position")) {
            camera.SetPosition(baseCameraPos);
            changed = true;
        }

        if (ImGui::DragFloat3("Forward", glm::value_ptr(cameraForward), 0.01f)) {
            camera.SetOrientation(cameraForward);
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##Forward")) {
            camera.SetOrientation(baseCameraForward);
            changed = true;
        }

        ImGui::Spacing();

        ImGui::SeparatorText("Lens");
        if (ImGui::SliderFloat("FOV", &fovDeg, 1.0f, 179.0f, "%.1f°")) {
            camera.SetFov(fovDeg);
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##Fov")) {
            camera.SetFov(baseFov);
            changed = true;
        }

        ImGui::Unindent();
        ImGui::TreePop();
    }

    return changed;
}
