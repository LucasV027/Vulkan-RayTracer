#pragma once

#include "Vulkan.h"

class DescriptorSetLayoutBuilder {
public:
    DescriptorSetLayoutBuilder& AddBinding(uint32_t binding,
                                           vk::DescriptorType type,
                                           vk::ShaderStageFlags stages,
                                           uint32_t count = 1);

    vk::DescriptorSetLayout Build(vk::Device device);

    void Clear();

private:
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

class DescriptorSetWriter {
public:
    DescriptorSetWriter& WriteBuffer(uint32_t binding,
                                     vk::Buffer buffer,
                                     vk::DeviceSize size,
                                     vk::DescriptorType type = vk::DescriptorType::eUniformBuffer,
                                     vk::DeviceSize offset = 0);

    DescriptorSetWriter& WriteImage(uint32_t binding,
                                    vk::ImageView imageView,
                                    vk::Sampler sampler,
                                    vk::ImageLayout layout,
                                    vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler);

    DescriptorSetWriter& WriteStorageImage(uint32_t binding,
                                           vk::ImageView imageView,
                                           vk::ImageLayout layout = vk::ImageLayout::eGeneral);

    void Update(vk::Device device, vk::DescriptorSet descriptorSet);
    void Clear();

private:
    std::vector<vk::WriteDescriptorSet> writes;
    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::DescriptorImageInfo> imageInfos;
};

