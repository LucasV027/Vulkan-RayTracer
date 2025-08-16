#pragma once
#include <memory>

#include "Camera.h"
#include "DirtySystem.h"
#include "Scene.h"

enum class DirtyFlags {
    Camera = 0,
    Scene = 1,
    Size = 2,
};

class Raytracer : public DirtySystem<DirtyFlags, 3> {
public:
    Raytracer(uint32_t width, uint32_t height);
    ~Raytracer() = default;

    void Update(uint32_t newWidth, uint32_t newHeight);

    Camera& GetCamera() { return *camera; }
    const Camera& GetCamera() const { return *camera; }
    const std::shared_ptr<Camera>& GetCameraRef() { return camera; }

    Scene& GetScene() { return scene; }
    const Scene& GetScene() const { return scene; }

    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

private:
    uint32_t width;
    uint32_t height;
    std::shared_ptr<Camera> camera;
    Scene scene;
};
