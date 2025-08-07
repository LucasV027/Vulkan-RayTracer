#pragma once

#include <cstdint>
#include <string>

#include "Raytracer/Raytracer.h"
#include "Renderer/ComputePipeline.h"
#include "Renderer/Renderer.h"
#include "Window/Window.h"
#include "Vulkan/VulkanContext.h"

class Application {
public:
    Application(const std::string& title, uint32_t width, uint32_t height);
    void Run();
    ~Application();

private:
    void Update(float dt) const;
    void Compute(float dt) const;
    void Render() const;

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<Raytracer> raytracer;
    std::shared_ptr<Renderer> renderer;
    std::unique_ptr<ComputePipeline> computePipeline;

    std::chrono::steady_clock::time_point lastFrameTime;
};
