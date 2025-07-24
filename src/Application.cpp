#include "Application.h"

#include <set>

#include "Utils.h"
#include "Log.h"

Application::Application() {
    try {
        window.Create(width, height, appName);
        renderer.Init(window);
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() {
    while (!window.ShouldClose()) {
        glfwPollEvents();
        renderer.RenderFrame();
    }
}

Application::~Application() {
    renderer.Cleanup();
}

