#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vulkan.h"

class Application {
public:
    Application();
    void Run();
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
    void CreateVertexBuffer();

    void Update();

    uint32_t AcquireNextImage();
    void Render(uint32_t swapChainIndex);
    void PresentImage(uint32_t swapChainIndex);

    void Cleanup();
    void VulkanCleanup();
    void GLFWCleanup() const;

private:
    std::string appName = "Vulkan-RayTracer";
    int width = 800;
    int height = 600;

    const std::vector<float> vertices = {
        0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
    };

    // GLFW
    GLFWwindow* window = nullptr;
    bool running = true;

    // Vulkan
    struct PerFrame {
        vk::CommandPool commandPool = nullptr;
        vk::CommandBuffer commandBuffer = nullptr;
        vk::Semaphore imageAvailable = nullptr;
        vk::Semaphore renderFinished = nullptr;
        vk::Fence inFlight = nullptr;

        void Destroy(vk::Device device) const;
    };

    struct {
        vk::Instance instance = nullptr;
        vk::DebugUtilsMessengerEXT debugCallback = nullptr;
        vk::SurfaceKHR surface = nullptr;
        vk::PhysicalDevice gpu = nullptr;
        vk::Device device = nullptr;

        vk::PipelineLayout graphicsPipelineLayout = nullptr;
        vk::Pipeline graphicsPipeline = nullptr;

        vk::Buffer vertexBuffer = nullptr;
        vk::DeviceMemory vertexBufferMemory = nullptr;

        vk::SwapchainKHR swapChain = nullptr;
        std::vector<vk::Image> swapChainImages;
        std::vector<vk::ImageView> swapChainImagesViews;
        std::vector<PerFrame> perFrames;

        struct {
            uint32_t width = 0;
            uint32_t height = 0;
            vk::Format format = vk::Format::eUndefined;
        } swapChainDimensions;

        uint32_t graphicsQueueIndex = -1;
        vk::Queue graphicsQueue = nullptr;

        uint32_t computeQueueIndex = -1;
        vk::Queue computeQueue = nullptr;
    } ctx;
};
