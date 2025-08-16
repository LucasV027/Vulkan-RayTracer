#include "Application.h"

#include <thread>
#include <chrono>

#include <imgui.h>

#include "Core/Log.h"
#include "UI/RaytracerUI.h"

Application::Application(const std::string& title, uint32_t width, uint32_t height) {
    try {
        window = std::make_shared<Window>(width, height, title);
        vulkanContext = std::make_shared<VulkanContext>(window);
        renderer = std::make_unique<Renderer>(window, vulkanContext);
        raytracer = std::make_unique<Raytracer>(width, height);
        cameraController = std::make_unique<CameraController>(window);
        cameraController->Register(raytracer->GetCameraRef());
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() const {
    using clock = std::chrono::high_resolution_clock;

    auto lastTick = clock::now();
    float dt = 0.0f;

    while (!window->ShouldClose()) {
        auto now = clock::now();
        std::chrono::duration<float> elapsed = now - lastTick;
        dt = elapsed.count();
        lastTick = now;

        Update(dt);
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

void Application::Update(const float dt) const {
    window->PollEvents();
    if (cameraController->Update(dt)) raytracer->SetDirty(DirtyFlags::Camera);
    raytracer->Update(window->GetWidth(), window->GetHeight());
    renderer->Update(*raytracer);
}

void Application::Render() const {
    renderer->Begin();
    DrawUI();
    renderer->Draw();
}
