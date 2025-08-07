#include "ComputePipeline.h"

#include "Vulkan/DescriptorSet.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context,
                                 const Raytracer& raytracer) :
    Pipeline(context),
    context(context),
    width(raytracer.GetWidth()),
    height(raytracer.GetHeight()) {
    CreateResources();
    CreateCommandPoolAndBuffer();
    CreateDescriptorSetLayout();
    CreateDescriptorSet();
    Pipeline::CreatePipelineLayout();
    CreatePipeline();
    ComputeGroupCount();
}

ComputePipeline::~ComputePipeline() {
    context->device.freeCommandBuffers(commandPool, commandBuffer);
    context->device.destroyCommandPool(commandPool);
}

void ComputePipeline::Dispatch() const {
    commandBuffer.reset({});

    commandBuffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    TransitionForCompute(commandBuffer);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet.get(), {});
    commandBuffer.dispatch(groupCountX, groupCountY, GROUP_COUNT_Z);
    TransitionForDisplay(commandBuffer);

    commandBuffer.end();

    const vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    context->graphicsQueue.submit(submitInfo);
    context->graphicsQueue.waitIdle();
}

void ComputePipeline::Upload(const Raytracer& raytracer) const {
    uniformsBuffer->Update(raytracer.GetUniforms());
}

void ComputePipeline::OnResize(const uint32_t newWidth, const uint32_t newHeight) {
    if (width == newWidth && height == newHeight) return;

    width = newWidth;
    height = newHeight;

    context->device.waitIdle();

    CreateResources();
    CreateDescriptorSet();
    ComputeGroupCount();
}

void ComputePipeline::CreateCommandPoolAndBuffer() {
    const vk::CommandPoolCreateInfo poolInfo = {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = context->graphicsQueueIndex,

    };

    commandPool = context->device.createCommandPool(poolInfo);

    const vk::CommandBufferAllocateInfo allocInfo = {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    commandBuffer = context->device.allocateCommandBuffers(allocInfo).front();
}

void ComputePipeline::CreateDescriptorSetLayout() {
    constexpr auto stage = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBuilder layoutBuilder;
    layoutBuilder.AddBinding(0, vk::DescriptorType::eUniformBuffer, stage)
                 .AddBinding(1, vk::DescriptorType::eStorageImage, stage)
                 .AddTo(vulkanContext->device, descriptorSetLayouts);
}

void ComputePipeline::CreateDescriptorSet() {
    descriptorSet = std::move(AllocateDescriptorSets()[0]);

    DescriptorSetWriter writer;
    writer.WriteBuffer(0, uniformsBuffer->GetHandle(), uniformsBuffer->GetSize())
          .WriteStorageImage(1, outputImageView.get())
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
                                          width,
                                          height,
                                          vk::Format::eR32G32B32A32Sfloat,
                                          vk::ImageUsageFlagBits::eStorage |
                                          vk::ImageUsageFlagBits::eSampled |
                                          vk::ImageUsageFlagBits::eTransferSrc);

    outputImageView = outputImage->CreateView();

    uniformsBuffer = std::make_unique<Buffer>(vulkanContext, sizeof(Uniforms), vk::BufferUsageFlagBits::eUniformBuffer);
}

void ComputePipeline::ComputeGroupCount() {
    groupCountX = (width + WORK_GROUP_SIZE_X - 1) / WORK_GROUP_SIZE_X;
    groupCountY = (height + WORK_GROUP_SIZE_Y - 1) / WORK_GROUP_SIZE_Y;
}

void ComputePipeline::TransitionForCompute(const vk::CommandBuffer cmd) const {
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

