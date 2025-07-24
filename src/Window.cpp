#include "Window.h"

Window::~Window() {
    glfwDestroyWindow(window);
    windowCount--;
    if (windowCount == 0) glfwTerminate();
}

void Window::Create(const uint32_t width, const uint32_t height, const std::string& title) {
    if (windowCount == 0) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) { throw std::runtime_error("Failed to create GLFW window"); }

    ++windowCount;
    glfwMakeContextCurrent(window);
}

void Window::PollEvents() const {
    glfwPollEvents();
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(window);
}

std::pair<uint32_t, uint32_t> Window::GetSize() const {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return std::make_pair(width, height);
}

std::pair<uint32_t, uint32_t> Window::GetFrameBufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return std::make_pair(width, height);
}

VkSurfaceKHR Window::CreateSurface(VkInstance instance) const {
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &rawSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface.");
    }

    return vk::SurfaceKHR(rawSurface);
}


