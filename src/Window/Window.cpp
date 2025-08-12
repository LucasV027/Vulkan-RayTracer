#include "Window.h"

#include <imgui.h>
#include <stdexcept>

Window::Window(const uint32_t width, const uint32_t height, const std::string& title) : width(width), height(height) {
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

    // Callbacks
    glfwSetWindowUserPointer(handle, this);

    glfwSetWindowSizeCallback(handle, [](GLFWwindow* window, const int width, const int height) {
        auto* _this = static_cast<Window*>(glfwGetWindowUserPointer(window));
        _this->width = width;
        _this->height = height;
    });

    glfwSetScrollCallback(handle, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto* _this = static_cast<Window*>(glfwGetWindowUserPointer(window));
        _this->scrollOffset = yOffset;
    });
}

Window::~Window() {
    glfwDestroyWindow(handle);
    windowCount--;
    if (windowCount == 0) glfwTerminate();
}

void Window::PollEvents() const {
    scrollOffset = 0.0;
    glfwPollEvents();
}

void Window::WaitEvents() const {
    scrollOffset = 0.0;
    glfwWaitEvents();
}

void Window::WaitWhileMinimized() const {
    while (IsMinimized()) {
        if (ShouldClose()) return;
        WaitEvents();
    }
}

uint32_t Window::GetWidth() const { return width; }
uint32_t Window::GetHeight() const { return height; }
const char* Window::GetTitle() const { return glfwGetWindowTitle(handle); }

GLFWwindow* Window::Handle() const { return handle; }

bool Window::ShouldClose() const { return glfwWindowShouldClose(handle); }
bool Window::IsMinimized() const { return width == 0 && height == 0; }

bool Window::IsKeyDown(const int key) const { return glfwGetKey(handle, key) == GLFW_PRESS; }

bool Window::IsMouseButtonDown(const int button) const {
    return !ImGui::GetIO().WantCaptureMouse && (glfwGetMouseButton(handle, button) == GLFW_PRESS);
}

std::pair<double, double> Window::GetMousePos() const {
    double xPos, yPos;
    glfwGetCursorPos(handle, &xPos, &yPos);
    return std::make_pair(xPos, yPos);
}

double Window::GetScrollOffset() const {
    return scrollOffset;
}

void Window::SetMouseLock(const bool locked) const {
    glfwSetInputMode(handle, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

