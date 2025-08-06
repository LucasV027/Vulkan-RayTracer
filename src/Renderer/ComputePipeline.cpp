#include "ComputePipeline.h"

#include "Raytracer/Data.h"
#include "Vulkan/DescriptorSet.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context,
                                 const std::shared_ptr<Raytracer>& raytracer) :
    Pipeline(context),
    raytracer(raytracer) {
    CreateResources();
    CreateDescriptorSetLayout();
    CreateDescriptorSet();
    Pipeline::CreatePipelineLayout();
    CreatePipeline();
}

void ComputePipeline::Record(const vk::CommandBuffer cb) const {
    const auto gcX = (raytracer->GetWidth() + WORK_GROUP_SIZE_X - 1) / WORK_GROUP_SIZE_X;
    const auto gcY = (raytracer->GetHeight() + WORK_GROUP_SIZE_Y - 1) / WORK_GROUP_SIZE_Y;
    constexpr auto gcZ = (1 + WORK_GROUP_SIZE_Z - 1) / WORK_GROUP_SIZE_Z;

    TransitionForCompute(cb);

    cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet.get(), {});
    cb.dispatch(gcX, gcY, gcZ);

    CopyResultToAcc(cb);
    TransitionForDisplay(cb);
}

void ComputePipeline::Resize() {
    CreateDescriptorSet();
}

void ComputePipeline::Update() const {
    uniformsBuffer->Update(CPU::ToGPU(raytracer->GetData()));
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
    writer.WriteBuffer(0, uniformsBuffer->GetHandle(), uniformsBuffer->GetSize())
          .WriteStorageImage(1, accumulationImageView.get())
          .WriteStorageImage(2, outputImageView.get())
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

void ComputePipeline::CreateResources() {
    outputImage = std::make_unique<Image>(vulkanContext,
                                          raytracer->GetWidth(),
                                          raytracer->GetHeight(),
                                          vk::Format::eR32G32B32A32Sfloat,
                                          vk::ImageUsageFlagBits::eStorage |
                                          vk::ImageUsageFlagBits::eSampled |
                                          vk::ImageUsageFlagBits::eTransferSrc);

    outputImageView = outputImage->CreateView();


    accumulationImage = std::make_unique<Image>(vulkanContext,
                                                raytracer->GetWidth(),
                                                raytracer->GetHeight(),
                                                vk::Format::eR32G32B32A32Sfloat,
                                                vk::ImageUsageFlagBits::eStorage |
                                                vk::ImageUsageFlagBits::eSampled |
                                                vk::ImageUsageFlagBits::eTransferDst);

    accumulationImageView = accumulationImage->CreateView();

    uniformsBuffer = std::make_unique<Buffer>(vulkanContext,
                                              sizeof(GPU::Data),
                                              vk::BufferUsageFlagBits::eUniformBuffer);
}

void ComputePipeline::TransitionForCompute(const vk::CommandBuffer cmd) const {
    accumulationImage->TransitionLayout(cmd,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eGeneral,
                                        vk::PipelineStageFlagBits2::eTopOfPipe,
                                        vk::PipelineStageFlagBits2::eComputeShader);

    outputImage->TransitionLayout(cmd,
                                  vk::ImageLayout::eUndefined,
                                  vk::ImageLayout::eGeneral,
                                  vk::PipelineStageFlagBits2::eTopOfPipe,
                                  vk::PipelineStageFlagBits2::eComputeShader);
}

void ComputePipeline::TransitionForDisplay(const vk::CommandBuffer cmd) const {
    outputImage->TransitionLayout(cmd,
                                  vk::ImageLayout::eGeneral,
                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                  vk::PipelineStageFlagBits2::eComputeShader,
                                  vk::PipelineStageFlagBits2::eFragmentShader);
}

void ComputePipeline::CopyResultToAcc(const vk::CommandBuffer cmd) const {
    accumulationImage->TransitionLayout(cmd,
                                        vk::ImageLayout::eGeneral,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        vk::PipelineStageFlagBits2::eComputeShader,
                                        vk::PipelineStageFlagBits2::eTransfer);

    outputImage->TransitionLayout(cmd,
                                  vk::ImageLayout::eGeneral,
                                  vk::ImageLayout::eTransferSrcOptimal,
                                  vk::PipelineStageFlagBits2::eComputeShader,
                                  vk::PipelineStageFlagBits2::eTransfer);

    const vk::ImageCopy copyRegion{
        .srcSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .dstSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .extent = {
            .width = outputImage->GetWidth(),
            .height = outputImage->GetHeight(),
            .depth = 1
        }
    };

    cmd.copyImage(outputImage->GetHandle(), vk::ImageLayout::eTransferSrcOptimal,
                  accumulationImage->GetHandle(), vk::ImageLayout::eTransferDstOptimal,
                  1, &copyRegion);

    accumulationImage->TransitionLayout(cmd,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        vk::ImageLayout::eGeneral,
                                        vk::PipelineStageFlagBits2::eTransfer,
                                        vk::PipelineStageFlagBits2::eComputeShader);

    outputImage->TransitionLayout(cmd,
                                  vk::ImageLayout::eTransferSrcOptimal,
                                  vk::ImageLayout::eGeneral,
                                  vk::PipelineStageFlagBits2::eTransfer,
                                  vk::PipelineStageFlagBits2::eComputeShader);
}
