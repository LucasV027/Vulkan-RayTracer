#pragma once
#include <memory>

#include "Camera.h"
#include "Scene.h"

class Raytracer {
public:
    Raytracer(uint32_t width, uint32_t height);
    ~Raytracer() = default;

    void DrawUI();
    void Update(uint32_t newWidth, uint32_t newHeight);

    const Camera& GetCamera() const { return *camera; }
    const Scene& GetScene() const { return scene; }
    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

    const std::shared_ptr<Camera>& GetCameraRef() { return camera; }

private:
    uint32_t width;
    uint32_t height;
    std::shared_ptr<Camera> camera;
    Scene scene;
};
