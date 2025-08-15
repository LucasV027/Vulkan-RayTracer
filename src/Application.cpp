#include "Application.h"

#include <imgui.h>
#include <thread>

#include "Core/Log.h"
#include "UI/RaytracerUI.h"

Application::Application(const std::string& title, uint32_t width, uint32_t height) {
    try {
        window = std::make_shared<Window>(width, height, title);
        vulkanContext = std::make_shared<VulkanContext>(window);
        renderer = std::make_unique<Renderer>(window, vulkanContext);
        raytracer = std::make_unique<Raytracer>(width, height);
        cameraController = std::make_unique<CameraController>(window, raytracer->GetCameraRef());
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() const {
    while (!window->ShouldClose()) {
        Update();
        Render();
    }
}

void Application::DrawUI() const {
    ImGui::Begin("[INFO]");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Window size: (%d, %d)", window->GetWidth(), window->GetHeight());
    UI::DrawRaytracer(*raytracer);
    ImGui::End();
}

void Application::Update() const {
    window->PollEvents();
    cameraController->Update(0.001f);
    raytracer->Update(window->GetWidth(), window->GetHeight());
    renderer->Update(*raytracer);
}

void Application::Render() const {
    renderer->Begin();
    DrawUI();
    renderer->Draw();
}
