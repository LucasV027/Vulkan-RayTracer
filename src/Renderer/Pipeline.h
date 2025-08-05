#pragma once

#include <vector>

#include "Vulkan.h"
#include "VulkanContext.h"

class Pipeline {
public:
    explicit Pipeline(const std::shared_ptr<VulkanContext>& context);
    virtual ~Pipeline();

    virtual void Record(vk::CommandBuffer cb) const = 0;

protected:
    virtual void CreatePipelineLayout();

    std::vector<vk::UniqueDescriptorSet> AllocateDescriptorSets();

protected:
    std::shared_ptr<VulkanContext> vulkanContext;

    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
};

