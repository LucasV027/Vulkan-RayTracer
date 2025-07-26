#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Renderer/Vulkan.h"

class Window {
public:
    Window() = default;
    ~Window();

    static std::shared_ptr<Window> Create(uint32_t width, uint32_t height, const std::string& title);

    void PollEvents() const;
    bool ShouldClose() const;
    std::pair<uint32_t, uint32_t> GetSize() const;
    std::pair<uint32_t, uint32_t> GetFrameBufferSize() const;
    const char* GetTitle() const;
    GLFWwindow* Handle() const;

    VkSurfaceKHR CreateSurface(VkInstance instance) const;

private:
    GLFWwindow* handle = nullptr;
    inline static int windowCount = 0;
};
