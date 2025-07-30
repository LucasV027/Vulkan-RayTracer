#pragma once

#include <memory>

#include "Swapchain.h"
#include "VulkanContext.h"

class ImGuiPipeline {
public:
    ImGuiPipeline(const std::shared_ptr<VulkanContext>& context,
                  const std::shared_ptr<Window>& window,
                  const std::shared_ptr<Swapchain>& swapchain);

    ~ImGuiPipeline();

    void Begin() const;
    void End() const;
    void Render(vk::CommandBuffer cb) const;

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<Window> window;
    std::shared_ptr<Swapchain> swapchain;

    mutable bool frame = false;
};
