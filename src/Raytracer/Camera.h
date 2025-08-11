#pragma once

#include <glm/glm.hpp>

#include "Core/Base.h"

struct LAYOUT_STD140 CameraData {
    glm::vec3 cameraPosition;
    PAD(1);

    glm::vec3 cameraForward;
    PAD(1);

    glm::vec3 cameraRight;
    PAD(1);

    glm::vec3 cameraUp;
    float fovRad;
};

class Camera {
public:
    explicit Camera(const glm::vec3& position = {0.f, 0.f, 0.f},
                    const glm::vec3& up = {0.f, 1.f, 0.f},
                    const glm::vec3& orientation = {0.f, 0.f, -1.f},
                    float fovDeg = 45.f);

    ~Camera() = default;

    void DrawUI();

    void SetPosition(const glm::vec3& newPosition);
    void SetOrientation(const glm::vec3& newOrientation);
    void SetFov(float newFovDeg);

    const CameraData& GetData() const { return cameraData; }
    float GetFovDeg() const { return fovDeg; }

    bool NeedsUpdate() const { return needsUpdate; }
    void ResetUpdate() const { needsUpdate = false; }

private:
    void UpdateVectors();

private:
    CameraData cameraData;
    float fovDeg;
    mutable bool needsUpdate = true;
};

