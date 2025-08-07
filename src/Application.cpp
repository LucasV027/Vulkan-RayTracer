#include "Application.h"

#include <imgui.h>
#include <thread>

#include "Core/Log.h"

Application::Application(const std::string& title, uint32_t width, uint32_t height) {
    try {
        window = std::make_shared<Window>(width, height, title);
        raytracer = std::make_shared<Raytracer>(width, height);
        vulkanContext = std::make_shared<VulkanContext>(window);
        computePipeline = std::make_unique<ComputePipeline>(vulkanContext, *raytracer);
        renderer = std::make_unique<Renderer>(window, vulkanContext);

        window->SetResizeCallback([&](const int w, const int h) {
            raytracer->Resize(w, h);
            computePipeline->OnResize(w, h);
            computePipeline->Upload(*raytracer);
        });

        lastFrameTime = std::chrono::steady_clock::now();
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() {
    while (!window->ShouldClose()) {
        auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - lastFrameTime).count();
        lastFrameTime = now;

        Update(dt);
        Compute(dt);
        Render();
    }
}

Application::~Application() {
    // explicit order deletion
    raytracer.reset();
    window.reset();
    renderer.reset();
    computePipeline.reset();
    vulkanContext.reset();
}

void Application::Update(const float dt) const {
    window->PollEvents();
    raytracer->Update();
}

void Application::Compute(const float dt) const {
    if (raytracer->NeedsUpload()) computePipeline->Upload(*raytracer);
    computePipeline->Dispatch();
}

void Application::Render() const {
    renderer->Begin();
    ImGui::Begin("[INFO]");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame index: %d", raytracer->GetFrameIndex());
    auto [width, height] = window->GetSize();
    ImGui::Text("Window size: (%d, %d)", width, height);
    ImGui::End();
    renderer->Draw(computePipeline->GetImageView());
}
