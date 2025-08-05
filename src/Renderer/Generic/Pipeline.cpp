#include "Pipeline.h"

Pipeline::Pipeline(const std::shared_ptr<VulkanContext>& context) : vulkanContext(context) {}

Pipeline::~Pipeline() {
    if (vulkanContext && vulkanContext->device) {
        if (pipeline) vulkanContext->device.destroyPipeline(pipeline);
        if (pipelineLayout) vulkanContext->device.destroyPipelineLayout(pipelineLayout);

        for (const auto dsl : descriptorSetLayouts)
            vulkanContext->device.destroyDescriptorSetLayout(dsl);
    }
};

void Pipeline::CreatePipelineLayout() {
    if (descriptorSetLayouts.empty()) {
        pipelineLayout = vulkanContext->device.createPipelineLayout({});
        return;
    }

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
    };

    pipelineLayout = vulkanContext->device.createPipelineLayout(pipelineLayoutInfo);
}

std::vector<vk::UniqueDescriptorSet> Pipeline::AllocateDescriptorSets() {
    const vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = vulkanContext->mainDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
    };

    return vulkanContext->device.allocateDescriptorSetsUnique(allocInfo);
}
