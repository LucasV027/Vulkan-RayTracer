#pragma once
#include <memory>

#include "Camera.h"
#include "DirtySystem.h"
#include "Scene.h"

enum class DirtyFlags {
    Camera = 0,
    Scene = 1,
};

class Raytracer : public DirtySystem<DirtyFlags, 2> {
public:
    Raytracer(uint32_t width, uint32_t height);
    ~Raytracer() = default;

    void Update(uint32_t newWidth, uint32_t newHeight);

    Camera& GetCamera() const { return *camera; }
    Scene& GetScene() const { return scene; }
    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

    const std::shared_ptr<Camera>& GetCameraRef() { return camera; }

private:
    uint32_t width;
    uint32_t height;
    std::shared_ptr<Camera> camera;
    mutable Scene scene;
};
