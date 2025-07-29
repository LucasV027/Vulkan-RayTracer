#include "ComputePipeline.h"

ComputePipeline::ComputePipeline(const vk::Device device,
                                 const vk::PhysicalDevice physicalDevice,
                                 const vk::DescriptorPool descriptorPool) : device(device),
                                                                            physicalDevice(physicalDevice) {
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreatePipeline();
    AllocateDescriptorSet(descriptorPool);
    CreateUniformBuffer();
    UpdateDescriptorSet();
}

ComputePipeline::~ComputePipeline() {
    if (uniformBuffer.memory) device.freeMemory(uniformBuffer.memory);
    if (uniformBuffer.buffer) device.destroyBuffer(uniformBuffer.buffer);
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
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet, {});
    cmd.dispatch(x, y, z);
}

void ComputePipeline::CreateDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eCompute
    };

    const vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding,
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

void ComputePipeline::AllocateDescriptorSet(vk::DescriptorPool descriptorPool) {
    const vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout,
    };

    descriptorSet = device.allocateDescriptorSets(allocInfo).front();
}

void ComputePipeline::CreateUniformBuffer() {
    uniformBuffer = vkHelpers::CreateBuffer(
        device,
        physicalDevice,
        sizeof(uint32_t),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
}

void ComputePipeline::UpdateDescriptorSet() const {
    vk::DescriptorBufferInfo bufferInfo{
        .buffer = uniformBuffer.buffer,
        .offset = 0,
        .range = sizeof(uint32_t),
    };

    const vk::WriteDescriptorSet writeDescriptorSet{
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &bufferInfo,
    };

    device.updateDescriptorSets(writeDescriptorSet, {});
}
