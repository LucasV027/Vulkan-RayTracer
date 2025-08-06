#pragma once

#include <cstdint>
#include <string>
#include <utility>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
public:
    Window(uint32_t width, uint32_t height, const std::string& title);
    ~Window();

    void PollEvents() const;
    void WaitEvents() const;
    void WaitWhileMinimized() const;

    std::pair<uint32_t, uint32_t> GetSize() const;
    const char* GetTitle() const;
    GLFWwindow* Handle() const;

    bool ShouldClose() const;
    bool IsMinimized() const;

private:
    GLFWwindow* handle = nullptr;
    inline static int windowCount = 0;
};
