#include "ComputePipeline.h"

ComputePipeline::ComputePipeline(const vk::Device device,
                                 const vk::PhysicalDevice physicalDevice,
                                 const vk::DescriptorPool descriptorPool) : device(device),
                                                                            physicalDevice(physicalDevice) {
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
    AllocateDescriptorSet(descriptorPool);
    CreateUniforms();
    UpdateDescriptorSet();
}

ComputePipeline::~ComputePipeline() {
    uniformBuffer.Destroy(device);
    accumulationImage.Destroy(device);
    resultImage.Destroy(device);

    if (pipeline) device.destroyPipeline(pipeline);
    if (pipelineLayout) device.destroyPipelineLayout(pipelineLayout);
    if (descriptorSetLayout) device.destroyDescriptorSetLayout(descriptorSetLayout);
}

void ComputePipeline::UpdateUniform(const uint32_t frameIndex) {
    uniformBuffer.Update(device, frameIndex);
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

void ComputePipeline::CreateDescriptorSetLayout() {
    std::array<vk::DescriptorSetLayoutBinding, 3> bindings;

    // UBO (binding = 0)
    bindings[0] = {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eCompute
    };

    // Storage image - accumulation (binding = 1)
    bindings[1] = {
        .binding = 1,
        .descriptorType = vk::DescriptorType::eStorageImage,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eCompute
    };

    // Storage image - result (binding = 2)
    bindings[2] = {
        .binding = 2,
        .descriptorType = vk::DescriptorType::eStorageImage,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eCompute
    };

    const vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
}

void ComputePipeline::CreatePipelineLayout() {
    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
    };

    pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
}

void ComputePipeline::CreatePipeline() {
    vk::UniqueShaderModule shaderModule = vkHelpers::CreateShaderModule(device, "../shaders/main.comp.spv");

    const vk::PipelineShaderStageCreateInfo shaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eCompute,
        .module = shaderModule.get(),
        .pName = "main",
    };

    const vk::ComputePipelineCreateInfo pipelineInfo{
        .stage = shaderStageInfo,
        .layout = pipelineLayout,
    };

    pipeline = device.createComputePipeline({}, pipelineInfo).value;
}

void ComputePipeline::AllocateDescriptorSet(const vk::DescriptorPool descriptorPool) {
    const vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout,
    };

    descriptorSet = device.allocateDescriptorSets(allocInfo).front();
}

void ComputePipeline::CreateUniforms() {
    uniformBuffer = vkHelpers::CreateBuffer(
        device,
        physicalDevice,
        sizeof(uint32_t),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );

    accumulationImage = vkHelpers::CreateStorageImage(
        device,
        physicalDevice,
        800, 600,
        vk::Format::eR32G32B32A32Sfloat
    );

    resultImage = vkHelpers::CreateStorageImage(
        device,
        physicalDevice,
        800, 600,
        vk::Format::eR32G32B32A32Sfloat
    );
}

void ComputePipeline::UpdateDescriptorSet() const {
    std::array<vk::WriteDescriptorSet, 3> writeDescriptorSets;

    // UBO
    vk::DescriptorBufferInfo bufferInfo{
        .buffer = uniformBuffer.buffer,
        .offset = 0,
        .range = sizeof(uint32_t),
    };

    writeDescriptorSets[0] = {
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &bufferInfo,
    };

    // Storage Image: accumulation
    vk::DescriptorImageInfo accumulationInfo{
        .imageView = accumulationImage.view,
        .imageLayout = vk::ImageLayout::eGeneral
    };

    writeDescriptorSets[1] = {
        .dstSet = descriptorSet,
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageImage,
        .pImageInfo = &accumulationInfo
    };

    // Storage Image: result
    vk::DescriptorImageInfo resultInfo{
        .imageView = resultImage.view,
        .imageLayout = vk::ImageLayout::eGeneral
    };

    writeDescriptorSets[2] = {
        .dstSet = descriptorSet,
        .dstBinding = 2,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageImage,
        .pImageInfo = &resultInfo
    };

    device.updateDescriptorSets(writeDescriptorSets, {});
}
