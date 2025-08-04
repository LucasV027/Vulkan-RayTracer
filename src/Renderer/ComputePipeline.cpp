#include "ComputePipeline.h"

#include "DescriptorSet.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context) : Pipeline(context) {
    CreateDescriptorSet();
    Pipeline::CreatePipelineLayout();
    CreatePipeline();
}

ComputePipeline::~ComputePipeline() {
    if (accumulationImageView) context->device.destroyImageView(accumulationImageView);
    if (resultImageView) context->device.destroyImageView(resultImageView);
}

void ComputePipeline::Record(const vk::CommandBuffer cb) const {
    Dispatch(cb, (800 + 15) / 16, 600, 1);
}

void ComputePipeline::Dispatch(const vk::CommandBuffer cmd,
                               const uint32_t x,
                               const uint32_t y,
                               const uint32_t z) const {
    accumulationImage->TransitionLayout(cmd,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eGeneral,
                                        vk::PipelineStageFlagBits2::eTopOfPipe,
                                        vk::PipelineStageFlagBits2::eComputeShader
    );


    resultImage->TransitionLayout(cmd,
                                  vk::ImageLayout::eUndefined,
                                  vk::ImageLayout::eGeneral,
                                  vk::PipelineStageFlagBits2::eTopOfPipe,
                                  vk::PipelineStageFlagBits2::eComputeShader
    );

    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet, {});
    cmd.dispatch(x, y, z);

    resultImage->TransitionLayout(cmd,
                                  vk::ImageLayout::eGeneral,
                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                  vk::PipelineStageFlagBits2::eComputeShader,
                                  vk::PipelineStageFlagBits2::eFragmentShader);
}

void ComputePipeline::CreateDescriptorSet() {
    constexpr auto stage = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBuilder layoutBuilder;
    layoutBuilder.AddBinding(0, vk::DescriptorType::eUniformBuffer, stage)
                 .AddBinding(1, vk::DescriptorType::eStorageImage, stage)
                 .AddBinding(2, vk::DescriptorType::eStorageImage, stage)
                 .AddTo(context->device, descriptorSetLayouts);

    descriptorSet = AllocateDescriptorSets()[0];

    uniformBuffer = std::make_unique<Buffer>(context, sizeof(uint32_t), vk::BufferUsageFlagBits::eUniformBuffer);

    constexpr vk::ImageUsageFlags usage =
        vk::ImageUsageFlagBits::eStorage |
        vk::ImageUsageFlagBits::eSampled |
        vk::ImageUsageFlagBits::eTransferSrc;

    accumulationImage = std::make_unique<Image>(context, 800, 600, vk::Format::eR32G32B32A32Sfloat, usage);
    accumulationImageView = accumulationImage->CreateView();
    resultImage = std::make_unique<Image>(context, 800, 600, vk::Format::eR32G32B32A32Sfloat, usage);
    resultImageView = resultImage->CreateView();


    DescriptorSetWriter writer;
    writer.WriteBuffer(0, uniformBuffer->GetHandle(), uniformBuffer->GetSize())
          .WriteStorageImage(1, accumulationImageView)
          .WriteStorageImage(2, resultImageView)
          .Update(context->device, descriptorSet);
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
