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
    std::string name = "Vulkan-RayTracer";

    GLFWwindow* window = nullptr;

    struct PerFrame {
        vk::CommandPool commandPool = nullptr;
        vk::CommandBuffer commandBuffer = nullptr;
        vk::Semaphore imageAvailable = nullptr;
        vk::Semaphore renderFinished = nullptr;
        vk::Fence inFlight = nullptr;

        void Destroy(vk::Device device) const;
    };

    const std::vector<float> vertices = {
        0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
    };

    struct {
        vk::Instance instance = nullptr;
        vk::DebugUtilsMessengerEXT debugCallback = nullptr;
        vk::SurfaceKHR surface = nullptr;
        vk::PhysicalDevice gpu = nullptr;
        vk::Device device = nullptr;

        struct {
            uint32_t queueIndex = -1;
            vk::Queue queue = nullptr;

            vk::PipelineLayout pipelineLayout;
            vk::Pipeline pipeline;

            vk::Buffer vertexBuffer = nullptr;
            vk::DeviceMemory vertexBufferMemory = nullptr;

            struct {
                vk::SwapchainKHR handle = nullptr;
                std::vector<vk::Image> images;
                std::vector<vk::ImageView> imagesViews;
                std::vector<PerFrame> perFrames;

                struct {
                    uint32_t width = 0;
                    uint32_t height = 0;
                    vk::Format format = vk::Format::eUndefined;
                } dimensions;
            } swapChain;
        } graphics;

        struct {
            uint32_t queueIndex = -1;
            vk::Queue queue = nullptr;
        } compute;
    } ctx;
};
