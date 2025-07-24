#pragma once

#include <string>

#include "Renderer.h"
#include "Vulkan.h"
#include "Window.h"

class Application {
public:
    Application();
    void Run();
    ~Application();

private:
    void Resize();

private:
    std::string appName = "Vulkan-RayTracer";
    int width = 800;
    int height = 600;
    bool shouldResize = false;

    Window window;
    Renderer renderer;
};
