#include "ComputePipeline.h"

#include "Vulkan/DescriptorSet.h"

ComputePipeline::ComputePipeline(const std::shared_ptr<VulkanContext>& context) :
    Pipeline(context),
    currentWidth(-1),
    currentHeight(-1) {
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
}

void ComputePipeline::Update(const Raytracer& raytracer) {
    if (raytracer.IsAnyDirty()) pushData.frameIndex = 0;

    if (raytracer.IsDirty(DirtyFlags::Size)) {
        currentWidth = raytracer.GetWidth();
        currentHeight = raytracer.GetHeight();

        vulkanContext->device.waitIdle();

        CreateResources();
        CreateDescriptorSet();
        ComputeGroupCount();

        raytracer.ClearDirty(DirtyFlags::Size);
    }

    if (raytracer.IsDirty(DirtyFlags::Camera)) {
        cameraBuffer->Update(raytracer.GetCamera().GetData());
        raytracer.ClearDirty(DirtyFlags::Camera);
    }

    if (raytracer.IsDirty(DirtyFlags::Scene)) {
        auto data = raytracer.GetScene().GetData();
        data.sphereCount = raytracer.GetScene().GetSpheres().size();
        sceneBuffer->Update(data);
        raytracer.ClearDirty(DirtyFlags::Scene);
    }

    if (raytracer.IsDirty(DirtyFlags::BVH)) {
        if (!raytracer.GetScene().GetBVH().triangles.empty()) {
            trianglesBuffer->Update(raytracer.GetScene().GetBVH().triangles);
            bvhNodesBuffer->Update(raytracer.GetScene().GetBVH().nodes);
        }
        raytracer.ClearDirty(DirtyFlags::BVH);
    }

    if (raytracer.IsDirty(DirtyFlags::Spheres)) {
        spheresBuffer->Update(raytracer.GetScene().GetSpheres());
        raytracer.ClearDirty(DirtyFlags::Spheres);
    }

    pushData.frameIndex++;
}

void ComputePipeline::Dispatch(const vk::CommandBuffer commandBuffer) const {
    TransitionForCompute(commandBuffer);

    if (sceneBuffer->ShouldUpload()) {
        sceneBuffer->Upload(commandBuffer, vk::PipelineStageFlagBits2::eComputeShader,
                            vk::AccessFlagBits2::eShaderRead);
    }

    if (trianglesBuffer->ShouldUpload()) {
        trianglesBuffer->Upload(commandBuffer, vk::PipelineStageFlagBits2::eComputeShader,
                                vk::AccessFlagBits2::eShaderRead);
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
                 .AddBinding(3, vk::DescriptorType::eStorageBuffer, stage)
                 .AddBinding(4, vk::DescriptorType::eUniformBuffer, stage)
                 .AddBinding(5, vk::DescriptorType::eUniformBuffer, stage)
                 .AddTo(vulkanContext->device, descriptorSetLayouts);
}

void ComputePipeline::CreateDescriptorSet() {
    descriptorSet = std::move(AllocateDescriptorSets()[0]);

    DescriptorSetWriter writer;
    writer.WriteBuffer(0, cameraBuffer->GetHandle(), cameraBuffer->GetSize())
          .WriteStorageImage(1, outputImageView.get())
          .WriteBuffer(2, sceneBuffer->GetHandle(), sceneBuffer->GetSize())
          .WriteBuffer(3, trianglesBuffer->GetHandle(), trianglesBuffer->GetSize(), vk::DescriptorType::eStorageBuffer)
          .WriteBuffer(4, bvhNodesBuffer->GetHandle(), bvhNodesBuffer->GetSize())
          .WriteBuffer(5, spheresBuffer->GetHandle(), spheresBuffer->GetSize())
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

    constexpr auto uniformBufferFlag = vk::BufferUsageFlagBits::eUniformBuffer;
    constexpr auto storageBufferFlag = vk::BufferUsageFlagBits::eStorageBuffer;

    // This function is called every time the window is resized.
    // Only size-dependent resources (e.g. outputImage) need to be recreated on each call.
    // Other resources (camera, scene, etc.) only need to be created once.
    // If they already exist, we can return early.
    if (cameraBuffer) return;

    // ---- Camera uniform buffer ---- //
    cameraBuffer = std::make_unique<Buffer>(vulkanContext, sizeof(CameraData), uniformBufferFlag);

    // ---- Triangles storage buffer (device local) ---- //
    constexpr vk::DeviceSize trianglesBufferSize = sizeof(Triangle) * Triangle::MAX_TRIANGLES;
    trianglesBuffer = std::make_unique<BufferWithStaging>(vulkanContext, trianglesBufferSize, storageBufferFlag);

    // ---- BVH Nodes uniform buffer ---- //
    constexpr vk::DeviceSize bvhNodesBufferSize = sizeof(BVH_FlattenNode) * BVH_FlattenNode::MAX_BVH_NODES;
    bvhNodesBuffer = std::make_unique<Buffer>(vulkanContext, bvhNodesBufferSize, uniformBufferFlag);

    // ---- Scene buffer (device local) ---- //
    sceneBuffer = std::make_unique<BufferWithStaging>(vulkanContext, sizeof(SceneData), uniformBufferFlag);

    // ---- Spheres buffer ---- //
    constexpr vk::DeviceSize spheresBufferSize = sizeof(Sphere) * Sphere::MAX_SPHERES;
    spheresBuffer = std::make_unique<Buffer>(vulkanContext, spheresBufferSize, uniformBufferFlag);
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

