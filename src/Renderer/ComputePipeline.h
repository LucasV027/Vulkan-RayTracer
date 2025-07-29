#pragma once

#include "Vulkan.h"

class ComputePipeline {
public:
    ComputePipeline(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DescriptorPool descriptorPool);
    ~ComputePipeline();

    void UpdateUniform(uint32_t frameIndex);
    void Dispatch(vk::CommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z) const;

private:
    void CreateDescriptorSetLayout();
    void CreatePipelineLayout();
    void CreatePipeline();
    void AllocateDescriptorSet(vk::DescriptorPool descriptorPool);
    void CreateUniformBuffer();
    void UpdateDescriptorSet() const;

private:
    vk::Device device;
    vk::PhysicalDevice physicalDevice;

    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;

    vkHelpers::AllocatedBuffer uniformBuffer;

    vk::PipelineLayout pipelineLayout;
    vk::Pipeline pipeline;
};
