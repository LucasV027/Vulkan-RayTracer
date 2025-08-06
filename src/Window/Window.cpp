#include "Window.h"

#include <stdexcept>

Window::Window(const uint32_t width, const uint32_t height, const std::string& title) {
    if (windowCount == 0) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    handle = glfwCreateWindow((int)width, (int)height, title.c_str(), nullptr, nullptr);
    if (!handle) { throw std::runtime_error("Failed to create GLFW window"); }

    ++windowCount;
    glfwMakeContextCurrent(handle);
}

Window::~Window() {
    glfwDestroyWindow(handle);
    windowCount--;
    if (windowCount == 0) glfwTerminate();
}

void Window::PollEvents() const { glfwPollEvents(); }
void Window::WaitEvents() const { glfwWaitEvents(); }

void Window::WaitWhileMinimized() const {
    while (IsMinimized()) {
        if (ShouldClose()) return;
        WaitEvents();
    }
}

std::pair<uint32_t, uint32_t> Window::GetSize() const {
    int width, height;
    glfwGetWindowSize(handle, &width, &height);
    return std::make_pair(width, height);
}

const char* Window::GetTitle() const { return glfwGetWindowTitle(handle); }

GLFWwindow* Window::Handle() const { return handle; }

bool Window::ShouldClose() const { return glfwWindowShouldClose(handle); }

bool Window::IsMinimized() const {
    auto [width, height] = GetSize();
    return width == 0 && height == 0;
}
