#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vulkan.h"

class Application {
public:
    Application();
    void Run() const;
    ~Application();

private:
    void InitWindow();

    void InitVulkan();
    void CreateVkInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();

    void Cleanup() const;

private:
    std::string name = "Vulkan-RayTracer";

    GLFWwindow* window = nullptr;

    struct {
        vk::Instance instance = nullptr;
        vk::DebugUtilsMessengerEXT debugCallback = nullptr;
        vk::SurfaceKHR surface = nullptr;
        vk::PhysicalDevice gpu = nullptr;
        int32_t graphicsQueueIndex = -1;
        vk::Device device = nullptr;
        vk::Queue queue = nullptr;
    } context;
};
