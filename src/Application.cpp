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
            raytracer->OnResize(w, h);
            computePipeline->OnResize(w, h);
            computePipeline->Upload(*raytracer);
        });

        start = std::chrono::steady_clock::now();
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() {
    while (!window->ShouldClose()) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
            start = std::chrono::steady_clock::now();
            computePerSecond = computeCount;
            computeCount = 0;
            renderPerSecond = renderCount;
            renderCount = 0;
        }

        Update();
        Compute();
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

void Application::Update() const {
    window->PollEvents();
    raytracer->Update();
}

void Application::Compute() {
    computePipeline->Upload(*raytracer);
    computePipeline->Dispatch();
    computeCount++;
}

void Application::Render() {
    renderer->Begin();
    ImGui::Begin("[INFO]");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("CPS: %d | RPS: %d", computePerSecond, renderPerSecond);
    ImGui::Text("Frame index: %d", raytracer->GetFrameIndex());
    auto [width, height] = window->GetSize();
    ImGui::Text("Window size: (%d, %d)", width, height);
    raytracer->RenderUI();
    ImGui::End();
    renderer->Draw(computePipeline->GetImageView());
    renderCount++;
}
