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

    bool recreateDescriptorSet = false;

    if (raytracer.IsDirty(DirtyFlags::Camera)) {
        cameraUBO->Update(raytracer.GetCamera().GetData());
        raytracer.ClearDirty(DirtyFlags::Camera);
    }

    if (raytracer.IsDirty(DirtyFlags::SceneData)) {
        sceneDataUBO->Update(raytracer.GetScene().GetSceneData());
        raytracer.ClearDirty(DirtyFlags::SceneData);
    }

    if (raytracer.IsDirty(DirtyFlags::Meshes)) {
        meshesSSBO->Update(raytracer.GetScene().GetMeshes());
        recreateDescriptorSet |= meshesSSBO->Changed();
        raytracer.ClearDirty(DirtyFlags::Meshes);
    }

    if (raytracer.IsDirty(DirtyFlags::Triangles)) {
        trianglesSSBO->Update(raytracer.GetScene().GetTriangles());
        recreateDescriptorSet |= trianglesSSBO->Changed();
        raytracer.ClearDirty(DirtyFlags::Triangles);
    }

    if (raytracer.IsDirty(DirtyFlags::BVH_Nodes)) {
        bvhNodesSSBO->Update(raytracer.GetScene().GetBVHNodes());
        recreateDescriptorSet |= bvhNodesSSBO->Changed();
        raytracer.ClearDirty(DirtyFlags::BVH_Nodes);
    }

    if (raytracer.IsDirty(DirtyFlags::Spheres)) {
        spheresSSBO->Update(raytracer.GetScene().GetSpheres());
        recreateDescriptorSet |= spheresSSBO->Changed();
        raytracer.ClearDirty(DirtyFlags::Spheres);
    }

    if (recreateDescriptorSet) {
        CreateDescriptorSet();
        spheresSSBO->ResetChanged();
    }

    pushData.frameIndex++;
}

void ComputePipeline::Dispatch(const vk::CommandBuffer commandBuffer) const {
    TransitionForCompute(commandBuffer);

    // Upload already check if its needed
    meshesSSBO->Upload(commandBuffer, vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderRead);
    trianglesSSBO->Upload(commandBuffer, vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderRead);
    bvhNodesSSBO->Upload(commandBuffer, vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderRead);
    spheresSSBO->Upload(commandBuffer, vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderRead);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushData), &pushData);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet.get(), {});
    commandBuffer.dispatch(groupCountX, groupCountY, groupCountZ);

    TransitionForDisplay(commandBuffer);
}

void ComputePipeline::CreateDescriptorSetLayout() {
    constexpr auto stage = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBuilder layoutBuilder;
    layoutBuilder.AddBinding(0, vk::DescriptorType::eStorageImage, stage)
                 .AddBinding(1, vk::DescriptorType::eUniformBuffer, stage)
                 .AddBinding(2, vk::DescriptorType::eUniformBuffer, stage)
                 .AddBinding(3, vk::DescriptorType::eStorageBuffer, stage)
                 .AddBinding(4, vk::DescriptorType::eStorageBuffer, stage)
                 .AddBinding(5, vk::DescriptorType::eStorageBuffer, stage)
                 .AddBinding(6, vk::DescriptorType::eStorageBuffer, stage)
                 .AddTo(vulkanContext->device, descriptorSetLayouts);
}

void ComputePipeline::CreateDescriptorSet() {
    descriptorSet = std::move(AllocateDescriptorSets()[0]);

    DescriptorSetWriter writer;
    writer.WriteStorageImage(0, outputImageView.get())
          .WriteBuffer(1, cameraUBO->GetHandle(), cameraUBO->GetSize())
          .WriteBuffer(2, sceneDataUBO->GetHandle(), sceneDataUBO->GetSize())
          .WriteBuffer(3, meshesSSBO->GetHandle(), meshesSSBO->GetSize(), vk::DescriptorType::eStorageBuffer)
          .WriteBuffer(4, trianglesSSBO->GetHandle(), trianglesSSBO->GetSize(), vk::DescriptorType::eStorageBuffer)
          .WriteBuffer(5, bvhNodesSSBO->GetHandle(), bvhNodesSSBO->GetSize(), vk::DescriptorType::eStorageBuffer)
          .WriteBuffer(6, spheresSSBO->GetHandle(), spheresSSBO->GetSize(), vk::DescriptorType::eStorageBuffer)
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


    // This function is called every time the window is resized.
    // Only size-dependent resources (e.g. outputImage) need to be recreated on each call.
    // Other resources (camera, scene, etc.) only need to be created once.
    // If they already exist, we can return early.
    if (cameraUBO) return;

    constexpr auto uniformBufferFlag = vk::BufferUsageFlagBits::eUniformBuffer;

    // ---- Binding 1 : Camera uniform buffer ---- //
    cameraUBO = std::make_unique<Buffer>(vulkanContext, sizeof(CameraData), uniformBufferFlag);

    // ---- Binding 2 : SceneData uniform buffer ---- //
    sceneDataUBO = std::make_unique<Buffer>(vulkanContext, sizeof(SceneData), uniformBufferFlag);

    // ---- Binding 3 : Meshes uniform buffer ---- //
    constexpr vk::DeviceSize meshesBufferSize = sizeof(Mesh) * 10;
    meshesSSBO = std::make_unique<StorageBuffer>(vulkanContext, meshesBufferSize);

    // ---- Binding 4: Triangles uniform buffer ---- //
    constexpr vk::DeviceSize trianglesBufferSize = sizeof(Triangle) * 10;
    trianglesSSBO = std::make_unique<StorageBuffer>(vulkanContext, trianglesBufferSize);

    // ---- Binding 5 : BVH Nodes uniform buffer ---- //
    constexpr vk::DeviceSize bvhNodesBufferSize = sizeof(BVH_FlattenNode) * 10;
    bvhNodesSSBO = std::make_unique<StorageBuffer>(vulkanContext, bvhNodesBufferSize);

    // ---- Binding 6 : Spheres uniform buffer ---- //
    constexpr vk::DeviceSize spheresBufferSize = sizeof(Sphere) * 10;
    spheresSSBO = std::make_unique<StorageBuffer>(vulkanContext, spheresBufferSize);
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

