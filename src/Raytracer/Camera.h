#pragma once

#include <glm/glm.hpp>

#include "ComputeData.h"

class Camera {
public:
    explicit Camera(const glm::vec3& position = {0.f, 0.f, 0.f},
                    const glm::vec3& up = {0.f, 1.f, 0.f},
                    const glm::vec3& orientation = {0.f, 0.f, -1.f},
                    float fovDeg = 90.f);

    ~Camera() = default;

    void SetPosition(const glm::vec3& newPosition);
    void SetOrientation(const glm::vec3& newOrientation);
    void SetFov(float newFovDeg);

    const CameraData& GetData() const { return cameraData; }
    float GetFovDeg() const { return fovDeg; }
    glm::vec3 GetPosition() const { return cameraData.cameraPosition; }
    glm::vec3 GetForward() const { return cameraData.cameraForward; }

private:
    void UpdateVectors();

private:
    CameraData cameraData;
    float fovDeg;
};

