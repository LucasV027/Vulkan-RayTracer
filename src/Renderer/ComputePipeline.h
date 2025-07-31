#pragma once

#include "Pipeline.h"
#include "Vulkan.h"
#include "VulkanContext.h"

class ComputePipeline final : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext>& context);
    ~ComputePipeline() override;

    void Dispatch(vk::CommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z) const;

private:
    void CreateDescriptorSet();
    void CreatePipelineLayout();
    void CreatePipeline();

private:
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;

    vkHelpers::AllocatedBuffer uniformBuffer;
    vkHelpers::AllocatedImage accumulationImage;
    vkHelpers::AllocatedImage resultImage;
};
