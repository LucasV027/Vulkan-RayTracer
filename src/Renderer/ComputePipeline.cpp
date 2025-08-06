#include "ComputePipeline.h"

#include "Vulkan/DescriptorSet.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context,
                                 const std::shared_ptr<Raytracer::Context>& rtContext) :
    Pipeline(context),
    rtContext(rtContext) {
    CreateDescriptorSetLayout();
    CreateDescriptorSet();
    Pipeline::CreatePipelineLayout();
    CreatePipeline();
}

void ComputePipeline::Record(const vk::CommandBuffer cb) const {
    const auto gcX = (rtContext->GetWidth() + WORK_GROUP_SIZE_X - 1) / WORK_GROUP_SIZE_X;
    const auto gcY = (rtContext->GetHeight() + WORK_GROUP_SIZE_Y - 1) / WORK_GROUP_SIZE_Y;
    constexpr auto gcZ = (1 + WORK_GROUP_SIZE_Z - 1) / WORK_GROUP_SIZE_Z;

    cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet.get(), {});
    cb.dispatch(gcX, gcY, gcZ);
}

void ComputePipeline::Resize() {
    CreateDescriptorSet();
}

void ComputePipeline::CreateDescriptorSetLayout() {
    constexpr auto stage = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBuilder layoutBuilder;
    layoutBuilder.AddBinding(0, vk::DescriptorType::eUniformBuffer, stage)
                 .AddBinding(1, vk::DescriptorType::eStorageImage, stage)
                 .AddBinding(2, vk::DescriptorType::eStorageImage, stage)
                 .AddTo(vulkanContext->device, descriptorSetLayouts);
}

void ComputePipeline::CreateDescriptorSet() {
    descriptorSet = std::move(AllocateDescriptorSets()[0]);

    DescriptorSetWriter writer;
    writer.WriteBuffer(0, rtContext->GetSceneBuffer()->GetHandle(), rtContext->GetSceneBuffer()->GetSize())
          .WriteStorageImage(1, rtContext->GetAccumulationImageView())
          .WriteStorageImage(2, rtContext->GetOutputImageView())
          .Update(vulkanContext->device, descriptorSet.get());
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
