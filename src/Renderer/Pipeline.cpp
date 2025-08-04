#include "Pipeline.h"

Pipeline::Pipeline(const std::shared_ptr<VulkanContext>& context) : context(context) {}

Pipeline::~Pipeline() {
    if (context && context->device) {
        if (pipeline) context->device.destroyPipeline(pipeline);
        if (pipelineLayout) context->device.destroyPipelineLayout(pipelineLayout);

        for (const auto dsl : descriptorSetLayouts)
            context->device.destroyDescriptorSetLayout(dsl);
    }
};

void Pipeline::CreatePipelineLayout() {
    if (descriptorSetLayouts.empty()) {
        pipelineLayout = context->device.createPipelineLayout({});
        return;
    }

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
    };

    pipelineLayout = context->device.createPipelineLayout(pipelineLayoutInfo);
}

std::vector<vk::DescriptorSet> Pipeline::AllocateDescriptorSets() {
    const vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = context->mainDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
    };

    return context->device.allocateDescriptorSets(allocInfo);
}
