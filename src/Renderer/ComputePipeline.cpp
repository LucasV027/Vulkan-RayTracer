#include "ComputePipeline.h"

#include "DescriptorSet.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context,
                                 const std::shared_ptr<RaytracingContext>& rtContext) :
    Pipeline(context),
    rtContext(rtContext) {
    CreateDescriptorSet();
    Pipeline::CreatePipelineLayout();
    CreatePipeline();
}

void ComputePipeline::Record(const vk::CommandBuffer cb) const {
    const auto gcX = (rtContext->GetWidth() + workgroupSizeX - 1) / workgroupSizeX;
    const auto gcY = (rtContext->GetHeight() + workgroupSizeY - 1) / workgroupSizeY;
    constexpr auto gcZ = (1 + workgroupSizeZ - 1) / workgroupSizeZ;

    cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet, {});
    cb.dispatch(gcX, gcY, gcZ);
}

void ComputePipeline::CreateDescriptorSet() {
    constexpr auto stage = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBuilder layoutBuilder;
    layoutBuilder.AddBinding(0, vk::DescriptorType::eUniformBuffer, stage)
                 .AddBinding(1, vk::DescriptorType::eStorageImage, stage)
                 .AddBinding(2, vk::DescriptorType::eStorageImage, stage)
                 .AddTo(vulkanContext->device, descriptorSetLayouts);

    descriptorSet = AllocateDescriptorSets()[0];

    DescriptorSetWriter writer;
    writer.WriteBuffer(0, rtContext->GetSceneBuffer()->GetHandle(), rtContext->GetSceneBuffer()->GetSize())
          .WriteStorageImage(1, rtContext->GetAccumulationImageView())
          .WriteStorageImage(2, rtContext->GetOutputImageView())
          .Update(vulkanContext->device, descriptorSet);
}

void ComputePipeline::CreatePipeline() {
    vk::UniqueShaderModule shaderModule = vkHelpers::CreateShaderModule(
        vulkanContext->device, "../shaders/main.comp.spv");

    const vk::PipelineShaderStageCreateInfo shaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eCompute,
        .module = shaderModule.get(),
        .pName = "main",
    };

    const vk::ComputePipelineCreateInfo pipelineInfo{
        .stage = shaderStageInfo,
        .layout = pipelineLayout,
    };

    pipeline = vulkanContext->device.createComputePipeline({}, pipelineInfo).value;
}
