#include "Camera.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

Camera::Camera(const glm::vec3& position, const glm::vec3& up, const glm::vec3& orientation) {
    cameraData.cameraPosition = position;
    cameraData.cameraUp = glm::normalize(up);
    cameraData.cameraForward = glm::normalize(orientation);
    cameraData.fov = glm::radians(BASE_FOV);

    UpdateVectors();
}

void Camera::SetPosition(const glm::vec3& newPosition) {
    cameraData.cameraPosition = newPosition;
}

void Camera::SetOrientation(const glm::vec3& newOrientation) {
    cameraData.cameraForward = glm::normalize(newOrientation);
    UpdateVectors();
}

void Camera::SetFov(const float newFov) {
    cameraData.fov = glm::radians(glm::clamp(newFov, 1.0f, 179.0f));
}

void Camera::DrawUI() {
    if (ImGui::TreeNode("Camera")) {
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
        static float rawFov = BASE_FOV;
        if (ImGui::SliderFloat("FOV", &rawFov, 1.0f, 179.0f, "%.1f°")) {
            SetFov(rawFov);
            needsUpdate = true;
        }

        ImGui::TreePop();
    }
}

void Camera::UpdateVectors() {
    // Calculate right vector
    cameraData.cameraRight = glm::normalize(glm::cross(cameraData.cameraForward, glm::vec3(0, 1, 0)));

    // Recalculate up vector to ensure orthogonality
    cameraData.cameraUp = glm::normalize(glm::cross(cameraData.cameraRight, cameraData.cameraForward));
}
