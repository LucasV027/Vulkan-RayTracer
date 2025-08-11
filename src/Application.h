#pragma once

#include <cstdint>
#include <string>

#include "Window/Window.h"
#include "Vulkan/VulkanContext.h"
#include "Renderer/Renderer.h"
#include "Raytracer/Raytracer.h"
#include "Controller/CameraController.h"

class Application {
public:
    Application(const std::string& title, uint32_t width, uint32_t height);
    void Run() const;
    ~Application() = default;

private:
    void DrawUI() const;
    void Update() const;
    void Render() const;

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<Renderer> renderer;

    std::unique_ptr<Raytracer> raytracer;
    std::unique_ptr<CameraController> cameraController;
};
