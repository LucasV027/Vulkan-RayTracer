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
    CameraController(const std::shared_ptr<Window>& window,
                     const std::shared_ptr<Camera>& camera,
                     const CameraControllerConfig& config = {});
    ~CameraController() = default;

    bool Update(float dt);

private:
    bool HandleKeyboard(float dt) const;
    bool HandleMouse(float dt);
    bool HandleScroll(float dt) const;

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<Camera> camera;

    CameraControllerConfig config;

    bool firstClick = true;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    float yaw = -90.0f;
    float pitch = 0.0f;
};

