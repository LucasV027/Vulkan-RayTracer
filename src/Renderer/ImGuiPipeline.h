#pragma once

#include <memory>

#include "VulkanContext.h"

class ImGuiPipeline {
public:
    explicit ImGuiPipeline(const std::shared_ptr<VulkanContext>& context,
                           const std::shared_ptr<Window>& window,
                           vk::Format format,
                           uint32_t imageCount);

    ~ImGuiPipeline();

    void Begin();
    void Render(vk::CommandBuffer cb, vk::ImageView imageView, uint32_t width, uint32_t height);

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<Window> window;
};
