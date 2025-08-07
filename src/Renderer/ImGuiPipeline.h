#pragma once

#include <memory>

#include "Window/Window.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/VulkanContext.h"

class ImGuiPipeline final : public Pipeline {
public:
    ImGuiPipeline(const std::shared_ptr<VulkanContext>& context,
                  const std::shared_ptr<Window>& window,
                  const std::shared_ptr<Swapchain>& swapchain);

    ~ImGuiPipeline() override;

    void Begin() const;
    void End() const;
    void Record(vk::CommandBuffer cb) const;

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<Swapchain> swapchain;

    mutable bool frame = false;
};
