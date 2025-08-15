#include "Camera.h"

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
