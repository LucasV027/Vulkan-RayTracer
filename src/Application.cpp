#include "Application.h"

#include <imgui.h>

#include "Utils.h"
#include "Log.h"

Application::Application() {
    try {
        window = std::make_shared<Window>(width, height, appName);
        vulkanContext = std::make_shared<VulkanContext>(window);
        rtContext = std::make_shared<RaytracingContext>(vulkanContext, RaytracingContext::Config{
                                                            .width = width,
                                                            .height = height
                                                        });
        renderer = std::make_unique<Renderer>(window, vulkanContext, rtContext);
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
        ImGui::Text("Frame index: %d", rtContext->GetFrameIndex());
        ImGui::Text("Window size: (%d, %d)", width, height);
        ImGui::End();
        renderer->Draw();
        rtContext->Update();
    }
}

Application::~Application() {
    // explicit order deletion
    window.reset();
    rtContext.reset();
    renderer.reset();
    vulkanContext.reset();
}
