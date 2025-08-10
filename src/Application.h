#pragma once

#include <cstdint>
#include <string>

#include "Raytracer/Camera.h"
#include "Raytracer/Scene.h"
#include "Renderer/Renderer.h"
#include "Window/Window.h"
#include "Vulkan/VulkanContext.h"

class Application {
public:
    Application(const std::string& title, uint32_t width, uint32_t height);
    void Run() const;
    ~Application();

private:
    void DrawUI() const;
    void Update() const;
    void Render() const;

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<Renderer> renderer;

    std::unique_ptr<Camera> camera;
    std::unique_ptr<Scene> scene;

    uint32_t width, height;
};
