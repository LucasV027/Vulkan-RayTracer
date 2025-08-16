#pragma once

#include <memory>

#include "Raytracer/Camera.h"
#include "Window/Window.h"

struct CameraControllerConfig {
    float baseSpeed = 4.0f;
    float speedFactor = 4.0f;
    float mouseSensitivity = 100.0f;
    float fovScrollSpeed = 4.0f;
    float fovMin = 1.0f;
    float fovMax = 179.0f;
};

class CameraController {
public:
    explicit CameraController(const std::shared_ptr<Window>& window,
                              const CameraControllerConfig& config = {});
    ~CameraController() = default;

    void Register(const std::shared_ptr<Camera>& camera);

    bool Update(float dt);

private:
    bool HandleKeyboard(Camera& camera, float dt) const;
    bool HandleMouse(Camera& camera, float dt);
    bool HandleScroll(Camera& camera, float dt) const;

private:
    std::shared_ptr<Window> window;
    std::weak_ptr<Camera> weakCamera;

    CameraControllerConfig config;

    bool firstClick = true;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    float yaw = -90.0f;
    float pitch = 0.0f;
};

