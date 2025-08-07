#pragma once

#include <memory>

#include "Vulkan/Base.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/VulkanContext.h"

class GraphicsPipeline final : public Pipeline {
public:
    GraphicsPipeline(const std::shared_ptr<VulkanContext>& context,
                     const std::shared_ptr<Swapchain>& swapchain);

    ~GraphicsPipeline() override = default;

    void Record(vk::CommandBuffer cb) const;

    void Resize();

    void SetImageView(vk::ImageView newImageView);

private:
    void CreateDescriptorSetLayout();
    void CreateDescriptorSet();
    void CreatePipeline();
    void CreateSampler();

private:
    std::shared_ptr<Swapchain> swapchain;

    vk::UniqueDescriptorSet descriptorSet;

    vk::ImageView imageView;
    vk::UniqueSampler sampler;
};
