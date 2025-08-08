#pragma once

#include <glm/glm.hpp>

#include "Data.h"

class Camera {
public:
    explicit Camera(const glm::vec3& position = {0.f, 0.f, 0.f},
                    const glm::vec3& up = {0.f, 1.f, 0.f},
                    const glm::vec3& orientation = {0.f, 0.f, -1.f}
    );

    void SetPosition(const glm::vec3& newPosition);
    void SetOrientation(const glm::vec3& newOrientation);
    void SetFov(float newFov);

    void DrawUI();

    const CameraData& GetData() const { return cameraData; }
    bool NeedsUpdate() const { return needsUpdate; }

    void ResetUpdate() const { needsUpdate = false; }

private:
    void UpdateVectors();

private:
    static constexpr float BASE_FOV = 45.0f;

    CameraData cameraData;
    mutable bool needsUpdate = true;
};

