#include "ComputePipeline.h"

#include "Vulkan/DescriptorSet.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context) :
    Pipeline(context),
    context(context),
    currentWidth(-1),
    currentHeight(-1) {
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
}

void ComputePipeline::Update(const Camera& camera, const Scene& scene, const uint32_t width, const uint32_t height) {
    bool resize = false;
    if (currentWidth != width || currentHeight != height) {
        currentWidth = width;
        currentHeight = height;

        context->device.waitIdle();

        CreateResources();
        CreateDescriptorSet();
        ComputeGroupCount();

        pushData.frameIndex = 0;
        resize = true;
    }

    if (camera.NeedsUpdate() || resize) {
        cameraBuffer->Update(camera.GetData());
        camera.ResetUpdate();
        pushData.frameIndex = 0;
    }

    if (scene.NeedsUpdate() || resize) {
        stagingBuffer->Update(scene.GetData());
        uploadStaging = true;
        scene.ResetUpdate();
        pushData.frameIndex = 0;
    }

    pushData.frameIndex++;
}

void ComputePipeline::Dispatch(const vk::CommandBuffer commandBuffer) const {
    TransitionForCompute(commandBuffer);

    if (uploadStaging) {
        constexpr vk::BufferCopy copyRegion{
            .size = sizeof(SceneData)
        };

        commandBuffer.copyBuffer(stagingBuffer->GetHandle(), sceneBuffer->GetHandle(), copyRegion);

        const vk::BufferMemoryBarrier2 barrier{
            .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
            .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eComputeShader,
            .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .buffer = sceneBuffer->GetHandle(),
            .offset = 0,
            .size = sizeof(SceneData)
        };

        const vk::DependencyInfo depInfo{
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &barrier
        };
        commandBuffer.pipelineBarrier2(depInfo);

        uploadStaging = false;
    }

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushData), &pushData);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet.get(), {});
    commandBuffer.dispatch(groupCountX, groupCountY, groupCountZ);

    TransitionForDisplay(commandBuffer);
}

void ComputePipeline::CreateDescriptorSetLayout() {
    constexpr auto stage = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBuilder layoutBuilder;
    layoutBuilder.AddBinding(0, vk::DescriptorType::eUniformBuffer, stage)
                 .AddBinding(1, vk::DescriptorType::eStorageImage, stage)
                 .AddBinding(2, vk::DescriptorType::eUniformBuffer, stage)
                 .AddTo(vulkanContext->device, descriptorSetLayouts);
}

void ComputePipeline::CreateDescriptorSet() {
    descriptorSet = std::move(AllocateDescriptorSets()[0]);

    DescriptorSetWriter writer;
    writer.WriteBuffer(0, cameraBuffer->GetHandle(), cameraBuffer->GetSize())
          .WriteStorageImage(1, outputImageView.get())
          .WriteBuffer(2, sceneBuffer->GetHandle(), sceneBuffer->GetSize(), vk::DescriptorType::eUniformBuffer)
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

void ComputePipeline::CreatePipelineLayout() {
    constexpr vk::PushConstantRange pushConstants{
        .stageFlags = vk::ShaderStageFlagBits::eCompute,
        .offset = 0,
        .size = sizeof(PushData)
    };

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstants,
    };

    pipelineLayout = vulkanContext->device.createPipelineLayout(pipelineLayoutInfo);
}

void ComputePipeline::CreateResources() {
    outputImage = std::make_unique<Image>(vulkanContext,
                                          currentWidth,
                                          currentHeight,
                                          vk::Format::eR32G32B32A32Sfloat,
                                          vk::ImageUsageFlagBits::eStorage |
                                          vk::ImageUsageFlagBits::eSampled |
                                          vk::ImageUsageFlagBits::eTransferSrc);

    outputImageView = outputImage->CreateView();

    cameraBuffer = std::make_unique<Buffer>(vulkanContext, sizeof(CameraData), vk::BufferUsageFlagBits::eUniformBuffer);

    sceneBuffer = std::make_unique<Buffer>(vulkanContext,
                                           sizeof(SceneData),
                                           vk::BufferUsageFlagBits::eTransferDst |
                                           vk::BufferUsageFlagBits::eUniformBuffer,
                                           vk::MemoryPropertyFlagBits::eDeviceLocal);

    stagingBuffer = std::make_unique<Buffer>(vulkanContext,
                                             sizeof(SceneData),
                                             vk::BufferUsageFlagBits::eTransferSrc,
                                             vk::MemoryPropertyFlagBits::eHostVisible |
                                             vk::MemoryPropertyFlagBits::eHostCoherent
    );
}

void ComputePipeline::ComputeGroupCount() {
    groupCountX = (currentWidth + WORK_GROUP_SIZE_X - 1) / WORK_GROUP_SIZE_X;
    groupCountY = (currentHeight + WORK_GROUP_SIZE_Y - 1) / WORK_GROUP_SIZE_Y;
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

