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
    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateGraphicsPipeline();

    void Cleanup() const;

private:
    std::string name = "Vulkan-RayTracer";

    GLFWwindow* window = nullptr;

    struct {
        vk::Instance instance = nullptr;
        vk::DebugUtilsMessengerEXT debugCallback = nullptr;
        vk::SurfaceKHR surface = nullptr;
        vk::PhysicalDevice gpu = nullptr;
        vk::Device device = nullptr;

        uint32_t graphicsQueueIndex = -1;
        vk::Queue graphicsQueue = nullptr;

        uint32_t computeQueueIndex = -1;
        vk::Queue computeQueue = nullptr;

        vk::SwapchainKHR swapChain = nullptr;
        std::vector<vk::Image> swapChainImages;
        std::vector<vk::ImageView> swapChainImagesViews;

        struct {
            uint32_t width = 0;
            uint32_t height = 0;
            vk::Format format = vk::Format::eUndefined;
        } swapChainDimensions;

        vk::PipelineLayout graphicsPipelineLayout;
        vk::Pipeline graphicsPipeline;
    } context;
};
