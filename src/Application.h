#pragma once

#include <cstdint>
#include <string>

#include "Renderer/Renderer.h"
#include "Window.h"
#include "Renderer/VulkanContext.h"

class Application {
public:
    Application(std::string title, uint32_t width, uint32_t height);
    void Run() const;
    ~Application();

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<Raytracer::Context> rtContext;
    std::shared_ptr<Renderer> renderer;
};
