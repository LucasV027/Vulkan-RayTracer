#include "Application.h"

#include <imgui.h>
#include <thread>

#include "Core/Log.h"
using namespace std::chrono_literals;

Application::Application(std::string title, uint32_t width, uint32_t height) {
    try {
        window = std::make_shared<Window>(width, height, title);
        raytracer = std::make_shared<Raytracer>(width, height);
        vulkanContext = std::make_shared<VulkanContext>(window);
        renderer = std::make_unique<Renderer>(window, vulkanContext, raytracer);
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() const {
    while (!window->ShouldClose()) {
        raytracer->Update();
        renderer->Update();
        window->PollEvents();
        renderer->Begin();
        ImGui::Begin("[INFO]");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame index: %d", raytracer->GetFrameIndex());
        auto [width, height] = window->GetSize();
        ImGui::Text("Window size: (%d, %d)", width, height);
        ImGui::Checkbox("Slow?", &slow);
        ImGui::End();
        renderer->Draw();
        if (slow) std::this_thread::sleep_for(10ms);
    }
}

Application::~Application() {
    // explicit order deletion
    window.reset();
    renderer.reset();
    raytracer.reset();
    vulkanContext.reset();
}
