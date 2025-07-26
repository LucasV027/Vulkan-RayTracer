#pragma once

#include "Vulkan.h"
#include "Window.h"

const std::vector QUAD = {
    -1.f, -1.f, 1.f, -1.f, 1.f, 1.f,
    -1.f, -1.f, 1.f, 1.f, -1.f, 1.f,
};

struct FrameContext {
    vk::CommandPool commandPool = nullptr;
    vk::CommandBuffer commandBuffer = nullptr;
    vk::UniqueSemaphore imageAvailable;
    vk::Semaphore renderFinished = nullptr;
    vk::Fence inFlight = nullptr;
    uint32_t index;

    void Init(vk::Device device, uint32_t graphicsQueueIndex);
    void Destroy(vk::Device device) const;
};

class Renderer {
public:
    explicit Renderer(const std::shared_ptr<Window>& window);
    ~Renderer();

    void Begin();
    void Draw();

private:
    FrameContext* BeginFrame();
    void Render(const FrameContext& fc) const;
    void SubmitUI(const FrameContext& fc) const;
    void Submit(const FrameContext& fc) const;
    void Present(const FrameContext& fc);

private:
    void Init();
    void InitImGUI();

    void Cleanup();
    void CleanupImGui() const;

    enum class AcquireError {
        Suboptimal,
        OutOfDate,
        Failed,
    };

    std::expected<uint32_t, AcquireError> AcquireNextImage();
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
        std::vector<FrameContext> perFrames;

        struct {
            uint32_t width = 0;
            uint32_t height = 0;
            vk::Format format = vk::Format::eUndefined;
        } swapChainDimensions;

        uint32_t graphicsQueueIndex = -1;
        vk::Queue graphicsQueue = nullptr;

        uint32_t computeQueueIndex = -1;
        vk::Queue computeQueue = nullptr;

        vk::DescriptorPool imguiDescriptorPool = nullptr;
    } ctx;
};
