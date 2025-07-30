#pragma once

#include "Vulkan.h"
#include "VulkanContext.h"

class ComputePipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext>& context);
    ~ComputePipeline();

    void UpdateUniform(uint32_t frameIndex);
    void Dispatch(vk::CommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z) const;

private:
    void CreateDescriptorSetLayout();
    void CreatePipelineLayout();
    void CreatePipeline();
    void AllocateDescriptorSet(vk::DescriptorPool descriptorPool);
    void CreateUniforms();
    void UpdateDescriptorSet() const;

private:
    std::shared_ptr<VulkanContext> context;

    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;

    vkHelpers::AllocatedBuffer uniformBuffer;
    vkHelpers::AllocatedImage accumulationImage;
    vkHelpers::AllocatedImage resultImage;

    vk::PipelineLayout pipelineLayout;
    vk::Pipeline pipeline;
};
