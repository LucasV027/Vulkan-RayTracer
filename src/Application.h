#pragma once

#include <cstdint>
#include <string>

#include "Renderer/Renderer.h"
#include "Window.h"
#include "Renderer/VulkanContext.h"

class Application {
public:
    Application();
    void Run() const;
    ~Application();

private:
    std::string appName = "Vulkan-RayTracer";
    uint32_t width = 800;
    uint32_t height = 600;

    std::shared_ptr<Window> window;
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<Renderer> renderer;
};
