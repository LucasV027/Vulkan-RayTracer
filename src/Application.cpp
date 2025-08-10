#include "Application.h"

#include <imgui.h>
#include <thread>

#include "Core/Log.h"

Application::Application(const std::string& title, uint32_t width, uint32_t height) :
    width(width),
    height(height) {
    try {
        window = std::make_shared<Window>(width, height, title);
        vulkanContext = std::make_shared<VulkanContext>(window);
        computePipeline = std::make_unique<ComputePipeline>(vulkanContext);
        renderer = std::make_unique<Renderer>(window, vulkanContext);
        camera = std::make_unique<Camera>();
        scene = std::make_unique<Scene>();

        window->SetResizeCallback([&](int w, int h) {
            this->width = w;
            this->height = h;
        });
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

Application::~Application() {
    // explicit order deletion
    window.reset();
    renderer.reset();
    computePipeline.reset();
    vulkanContext.reset();
}

void Application::DrawUI() const {
    ImGui::Begin("[INFO]");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame index: %d", computePipeline->GetFrameIndex());
    ImGui::Text("Window size: (%d, %d)", width, height);
    camera->DrawUI();
    scene->DrawUI();
    ImGui::End();
}

void Application::Update() const {
    window->PollEvents();
    computePipeline->Update(*camera, width, height);
    computePipeline->Dispatch();
}

void Application::Render() const {
    renderer->Begin();
    DrawUI();
    renderer->Draw(computePipeline->GetImageView());
}
