#pragma once

#include <string>

#include "Renderer.h"
#include "Vulkan.h"
#include "Window.h"

class Application {
public:
    Application();
    void Run() const;
    ~Application() = default;

private:
    std::string appName = "Vulkan-RayTracer";
    uint32_t width = 800;
    uint32_t height = 600;

    std::shared_ptr<Window> window;
    std::shared_ptr<Renderer> renderer;
};
