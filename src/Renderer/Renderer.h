#pragma once

#include "ComputePipeline.h"
#include "GraphicsPipeline.h"
#include "ImGuiPipeline.h"
#include "Vulkan.h"
#include "VulkanContext.h"

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
    explicit Renderer(const std::shared_ptr<VulkanContext>& context, const std::shared_ptr<Window>& window);
    ~Renderer();

    void Begin() const;
    void Draw();

private:
    FrameContext* BeginFrame();
    void Submit(const FrameContext& fc) const;
    void Present(const FrameContext& fc);

private:
    void Cleanup();

    enum class AcquireError {
        Suboptimal,
        OutOfDate,
        Failed,
    };

    std::expected<uint32_t, AcquireError> AcquireNextImage();
    bool PresentImage(uint32_t swapChainIndex);

    void Resize();

    void CreateSwapChain();

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<Window> window;
    std::unique_ptr<ComputePipeline> computePipeline;
    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    std::unique_ptr<ImGuiPipeline> uiPipeline;

    vk::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::ImageView> swapChainImagesViews;
    std::vector<FrameContext> perFrames;

    struct {
        uint32_t width = 0;
        uint32_t height = 0;
        vk::Format format = vk::Format::eUndefined;
    } swapChainDimensions;
};
