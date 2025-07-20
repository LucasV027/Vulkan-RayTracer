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

    struct PerFrame {
        vk::CommandPool commandPool;
        vk::CommandBuffer commandBuffer;
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
