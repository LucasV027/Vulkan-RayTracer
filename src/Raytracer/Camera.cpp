#include "Camera.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

Camera::Camera(const glm::vec3& position,
               const glm::vec3& up,
               const glm::vec3& orientation,
               const float fovDeg) :
    fovDeg(fovDeg) {
    cameraData.cameraPosition = position;
    cameraData.cameraUp = glm::normalize(up);
    cameraData.cameraForward = glm::normalize(orientation);
    cameraData.fovRad = glm::radians(fovDeg);

    UpdateVectors();
}

void Camera::DrawUI() {
    static float baseFov = fovDeg;
    static glm::vec3 baseCameraPos = cameraData.cameraPosition;
    static glm::vec3 baseCameraForward = cameraData.cameraForward;

    if (ImGui::TreeNode("Camera")) {
        ImGui::Indent();

        ImGui::SeparatorText("Transform");
        if (ImGui::DragFloat3("Position", glm::value_ptr(cameraData.cameraPosition), 0.1f)) {
            SetPosition(cameraData.cameraPosition);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##Position")) {
            SetPosition(baseCameraPos);
        }

        if (ImGui::DragFloat3("Forward", glm::value_ptr(cameraData.cameraForward), 0.01f)) {
            SetOrientation(cameraData.cameraForward);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##Forward")) {
            SetOrientation(baseCameraForward);
        }

        ImGui::Spacing();

        ImGui::SeparatorText("Lens");
        if (ImGui::SliderFloat("FOV", &fovDeg, 1.0f, 179.0f, "%.1f°")) {
            SetFov(fovDeg);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##Fov")) {
            SetFov(baseFov);
        }

        ImGui::Unindent();
        ImGui::TreePop();
    }
}

void Camera::SetPosition(const glm::vec3& newPosition) {
    cameraData.cameraPosition = newPosition;
    needsUpdate = true;
}

void Camera::SetOrientation(const glm::vec3& newOrientation) {
    cameraData.cameraForward = glm::normalize(newOrientation);
    UpdateVectors();
    needsUpdate = true;
}

void Camera::SetFov(const float newFovDeg) {
    fovDeg = glm::clamp(newFovDeg, 1.0f, 179.0f);
    cameraData.fovRad = glm::radians(fovDeg);
    needsUpdate = true;
}

void Camera::UpdateVectors() {
    cameraData.cameraRight = glm::normalize(glm::cross(cameraData.cameraForward, glm::vec3(0, 1, 0)));
    cameraData.cameraUp = glm::normalize(glm::cross(cameraData.cameraRight, cameraData.cameraForward));
}
