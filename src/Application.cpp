#include "Application.h"

#include <imgui.h>

#include "Utils.h"
#include "Log.h"

Application::Application() {
    try {
        window = Window::Create(width, height, appName);
        context = std::make_shared<VulkanContext>(window);
        renderer = std::make_unique<Renderer>(context, window);
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() const {
    while (!window->ShouldClose()) {
        window->PollEvents();
        renderer->Begin();
        ImGui::Begin("[INFO]");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Window size: (%d, %d)", width, height);
        ImGui::End();
        renderer->Draw();
    }
}

Application::~Application() {
    // explicit order deletion
    window.reset();
    renderer.reset();
    context.reset();
}
