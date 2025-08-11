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
    if (ImGui::TreeNode("Camera")) {
        ImGui::NewLine();
        // Position controls
        if (ImGui::DragFloat3("Position", glm::value_ptr(cameraData.cameraPosition), 0.1f)) {
            needsUpdate = true;
        }

        // Forward vector control
        glm::vec3 forward = cameraData.cameraForward;
        if (ImGui::DragFloat3("Forward", glm::value_ptr(forward), 0.01f)) {
            SetOrientation(forward);
            needsUpdate = true;
        }

        // FOV control
        if (ImGui::SliderFloat("FOV", &fovDeg, 1.0f, 179.0f, "%.1f°")) {
            SetFov(fovDeg);
            needsUpdate = true;
        }

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
