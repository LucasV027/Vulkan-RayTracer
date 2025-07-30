#pragma once

#include "ComputePipeline.h"
#include "GraphicsPipeline.h"
#include "ImGuiPipeline.h"
#include "Swapchain.h"
#include "Vulkan.h"
#include "VulkanContext.h"

class Renderer {
public:
    explicit Renderer(const std::shared_ptr<VulkanContext>& context,
                      const std::shared_ptr<Window>& window);
    ~Renderer();

    void Begin() const;
    void Draw();

private:
    FrameContext* BeginFrame() const;
    void Submit(const FrameContext& fc) const;
    void Present(const FrameContext& fc) const;

private:
    enum class AcquireError {
        Suboptimal,
        OutOfDate,
        Failed,
    };

    std::expected<FrameContext* , AcquireError> AcquireNextImage() const;

    void Resize() const;

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<Window> window;
    std::shared_ptr<Swapchain> swapchain;
    std::unique_ptr<ComputePipeline> computePipeline;
    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    std::unique_ptr<ImGuiPipeline> uiPipeline;
};
