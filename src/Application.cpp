#include "Application.h"

#include <set>

#include "Utils.h"
#include "Log.h"

Application::Application() {
    try {
        window = Window::Create(width, height, appName);
        renderer = std::make_unique<Renderer>(window);
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() const {
    while (!window->ShouldClose()) {
        glfwPollEvents();
        renderer->RenderFrame();
    }
}

