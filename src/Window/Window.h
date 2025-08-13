#pragma once

#include <cstdint>
#include <string>
#include <utility>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vulkan/Base.h"

class Window {
public:
    Window(uint32_t width, uint32_t height, const std::string& title);
    ~Window();

    // Poll / Wait events
    void PollEvents() const;
    void WaitEvents() const;
    void WaitWhileMinimized() const;

    // Getters
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    const char* GetTitle() const;

    GLFWwindow* Handle() const;

    bool ShouldClose() const;
    bool IsMinimized() const;

    // Input
    bool IsKeyDown(int key) const;
    bool IsMouseButtonDown(int button) const;
    std::pair<double, double> GetMousePos() const;
    double GetScrollOffset() const;

    void SetMouseLock(bool locked) const;

    // Vulkan
    vk::SurfaceKHR CreateSurface(vk::Instance instance) const;
    std::vector<const char*> GetRequiredSurfaceExtensions() const;

private:
    GLFWwindow* handle = nullptr;
    uint32_t width, height;
    mutable double scrollOffset;

    inline static int windowCount = 0;
};
