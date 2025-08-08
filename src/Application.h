#pragma once

#include <cstdint>
#include <string>

#include "Raytracer/Camera.h"
#include "Renderer/ComputePipeline.h"
#include "Renderer/Renderer.h"
#include "Window/Window.h"
#include "Vulkan/VulkanContext.h"

class Application {
public:
    Application(const std::string& title, uint32_t width, uint32_t height);
    void Run() const;
    ~Application();

private:
    void Update() const;
    void Render() const;

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<Renderer> renderer;
    std::unique_ptr<ComputePipeline> computePipeline;
    std::unique_ptr<Camera> camera;

    uint32_t width, height;
};
