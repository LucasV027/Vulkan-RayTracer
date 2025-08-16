#include "CameraController.h"

CameraController::CameraController(const std::shared_ptr<Window>& window,
                                   const std::shared_ptr<Camera>& camera,
                                   const CameraControllerConfig& config) :
    window(window),
    camera(camera),
    config(config) {}

bool CameraController::Update(const float dt) {
    bool changed = false;
    changed |= HandleKeyboard(dt);
    changed |= HandleMouse(dt);
    changed |= HandleScroll(dt);
    return changed;
}

bool CameraController::HandleKeyboard(const float dt) const {
    const auto& data = camera->GetData();
    glm::vec3 position = data.cameraPosition;

    const auto forward = glm::normalize(data.cameraForward);
    const auto right = glm::normalize(data.cameraRight);
    constexpr auto up = glm::vec3(0.0f, 1.0f, 0.0f);

    const auto forwardHorizontal = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
    const auto rightHorizontal = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

    float speed = config.baseSpeed * dt;
    if (window->IsKeyDown(GLFW_KEY_LEFT_SHIFT)) speed *= config.speedFactor;

    if (window->IsKeyDown(GLFW_KEY_W)) position += forwardHorizontal * speed;
    if (window->IsKeyDown(GLFW_KEY_S)) position -= forwardHorizontal * speed;
    if (window->IsKeyDown(GLFW_KEY_D)) position += rightHorizontal * speed;
    if (window->IsKeyDown(GLFW_KEY_A)) position -= rightHorizontal * speed;
    if (window->IsKeyDown(GLFW_KEY_LEFT_CONTROL)) position += up * speed;
    if (window->IsKeyDown(GLFW_KEY_SPACE)) position -= up * speed;

    if (glm::length(position - data.cameraPosition) > 0.001f) {
        camera->SetPosition(position);
        return true;
    }
    return false;
}

bool CameraController::HandleMouse(const float dt) {
    if (!window->IsMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
        window->SetMouseLock(false);
        firstClick = true;
        return false;
    }

    window->SetMouseLock(true);
    auto [xPos, yPos] = window->GetMousePos();

    if (firstClick) {
        lastMouseX = xPos;
        lastMouseY = yPos;
        firstClick = false;
    }

    double xOffset = xPos - lastMouseX;
    double yOffset = yPos - lastMouseY;

    lastMouseX = xPos;
    lastMouseY = yPos;

    xOffset *= config.mouseSensitivity * dt;
    yOffset *= config.mouseSensitivity * dt;

    yaw += static_cast<float>(xOffset);
    pitch += static_cast<float>(yOffset);

    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    if (yaw > 360.0f) yaw -= 360.0f;
    if (yaw < 0.0f) yaw += 360.0f;

    const float yawRad = glm::radians(yaw);
    const float pitchRad = glm::radians(pitch);

    glm::vec3 newForward;
    newForward.x = cosf(yawRad) * cosf(pitchRad);
    newForward.y = sinf(pitchRad);
    newForward.z = sinf(yawRad) * cosf(pitchRad);

    camera->SetOrientation(glm::normalize(newForward));
    return true;
}

bool CameraController::HandleScroll(const float dt) const {
    const auto scroll = window->GetScrollOffset();
    if (scroll == 0.0) return false;

    const auto& currentFov = camera->GetFovDeg();
    float newFov = currentFov - static_cast<float>(scroll) * config.fovScrollSpeed;
    newFov = glm::clamp(newFov, config.fovMin, config.fovMax);

    if (abs(newFov - currentFov) > 0.1f) {
        camera->SetFov(newFov);
        return true;
    }
    return false;
}
