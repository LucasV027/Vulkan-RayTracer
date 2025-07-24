#pragma once

#include "Vulkan.h"
#include "Window.h"

const std::vector<float> vertices = {
    0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
};

class Renderer {
public:
    explicit Renderer(const std::shared_ptr<Window>& window);
    ~Renderer();

    void RenderFrame();

private:
    void Init();
    void Cleanup();

    enum class AcquireError {
        Suboptimal,
        OutOfDate,
        Failed,
    };

    std::expected<uint32_t, AcquireError> AcquireNextImage();
    void RenderTriangle(uint32_t swapChainIndex);
    bool PresentImage(uint32_t swapChainIndex);

    void Resize();

    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateGraphicsPipeline();
    void CreateVertexBuffer();

private:
    std::shared_ptr<Window> windowRef;

    struct PerFrame {
        vk::CommandPool commandPool = nullptr;
        vk::CommandBuffer commandBuffer = nullptr;
        vk::UniqueSemaphore imageAvailable;
        vk::Semaphore renderFinished = nullptr;
        vk::Fence inFlight = nullptr;

        void Init(vk::Device device, uint32_t graphicsQueueIndex);
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
