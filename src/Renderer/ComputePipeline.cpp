#include "ComputePipeline.h"

#include "DescriptorSet.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context) : Pipeline(context) {
    CreateDescriptorSet();
    CreatePipelineLayout();
    CreatePipeline();
}

ComputePipeline::~ComputePipeline() {
    accumulationImage.Destroy(context->device);
    resultImage.Destroy(context->device);

    if (descriptorSetLayout) context->device.destroyDescriptorSetLayout(descriptorSetLayout);
}

void ComputePipeline::Dispatch(const vk::CommandBuffer cmd,
                               const uint32_t x,
                               const uint32_t y,
                               const uint32_t z) const {
    vkHelpers::TransitionImageLayout(
        cmd,
        accumulationImage.image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eGeneral,
        {},
        vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderRead,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eComputeShader
    );

    vkHelpers::TransitionImageLayout(
        cmd,
        resultImage.image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eGeneral,
        {},
        vk::AccessFlagBits2::eShaderWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eComputeShader
    );

    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet, {});
    cmd.dispatch(x, y, z);

    vkHelpers::TransitionImageLayout(
        cmd,
        resultImage.image,
        vk::ImageLayout::eGeneral,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::AccessFlagBits2::eShaderWrite,
        vk::AccessFlagBits2::eShaderRead,
        vk::PipelineStageFlagBits2::eComputeShader,
        vk::PipelineStageFlagBits2::eFragmentShader
    );
}

void ComputePipeline::CreateDescriptorSet() {
    constexpr auto stage = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBuilder layoutBuilder;
    descriptorSetLayout = layoutBuilder
                          .AddBinding(0, vk::DescriptorType::eUniformBuffer, stage)
                          .AddBinding(1, vk::DescriptorType::eStorageImage, stage)
                          .AddBinding(2, vk::DescriptorType::eStorageImage, stage)
                          .Build(context->device);

    const vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = context->mainDescriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };

    descriptorSet = context->device.allocateDescriptorSets(allocInfo)[0];

    uniformBuffer = std::make_unique<Buffer>(context, sizeof(uint32_t), vk::BufferUsageFlagBits::eUniformBuffer);

    accumulationImage = vkHelpers::CreateStorageImage(
        context->device,
        context->physicalDevice,
        800, 600,
        vk::Format::eR32G32B32A32Sfloat
    );

    resultImage = vkHelpers::CreateStorageImage(
        context->device,
        context->physicalDevice,
        800, 600,
        vk::Format::eR32G32B32A32Sfloat
    );


    DescriptorSetWriter writer;
    writer.WriteBuffer(0, uniformBuffer->GetHandle(), uniformBuffer->GetSize())
          .WriteStorageImage(1, accumulationImage.view)
          .WriteStorageImage(2, resultImage.view)
          .Update(context->device, descriptorSet);
}

void ComputePipeline::CreatePipelineLayout() {
    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
    };

    pipelineLayout = context->device.createPipelineLayout(pipelineLayoutInfo);
}

void ComputePipeline::CreatePipeline() {
    vk::UniqueShaderModule shaderModule = vkHelpers::CreateShaderModule(context->device, "../shaders/main.comp.spv");

    const vk::PipelineShaderStageCreateInfo shaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eCompute,
        .module = shaderModule.get(),
        .pName = "main",
    };

    const vk::ComputePipelineCreateInfo pipelineInfo{
        .stage = shaderStageInfo,
        .layout = pipelineLayout,
    };

    pipeline = context->device.createComputePipeline({}, pipelineInfo).value;
}
