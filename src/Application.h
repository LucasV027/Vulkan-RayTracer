#pragma once

#include <cstdint>
#include <string>

#include "Window/Window.h"
#include "Vulkan/VulkanContext.h"
#include "Renderer/Renderer.h"
#include "Raytracer/Raytracer.h"
#include "Controller/CameraController.h"
#include "UI/ApplicationUI.h"

class Application {
public:
    Application(const std::string& title, uint32_t width, uint32_t height);
    void Run();
    ~Application() = default;

private:
    void Update(float dt) const;
    void Render();

    friend void UI::DrawApplication(Application& app);

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<Renderer> renderer;

    std::unique_ptr<Raytracer> raytracer;
    std::unique_ptr<CameraController> cameraController;
};
