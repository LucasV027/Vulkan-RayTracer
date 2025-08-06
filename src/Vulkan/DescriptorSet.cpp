#include "DescriptorSet.h"

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::AddBinding(const uint32_t binding,
                                                                   const vk::DescriptorType type,
                                                                   const vk::ShaderStageFlags stages,
                                                                   const uint32_t count) {
    bindings.push_back({
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = count,
        .stageFlags = stages
    });
    return *this;
}

vk::DescriptorSetLayout DescriptorSetLayoutBuilder::Build(const vk::Device device) {
    const vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    return device.createDescriptorSetLayout(createInfo);
}

void DescriptorSetLayoutBuilder::AddTo(const vk::Device device, std::vector<vk::DescriptorSetLayout>& layouts) {
    const vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    layouts.emplace_back(device.createDescriptorSetLayout(createInfo));
}

void DescriptorSetLayoutBuilder::Clear() { bindings.clear(); }

DescriptorSetWriter& DescriptorSetWriter::WriteBuffer(const uint32_t binding,
                                                      const vk::Buffer buffer,
                                                      const vk::DeviceSize size,
                                                      const vk::DescriptorType type,
                                                      const vk::DeviceSize offset) {
    bufferInfos.push_back({
        .buffer = buffer,
        .offset = offset,
        .range = size
    });

    writes.push_back({
        .sType = vk::StructureType::eWriteDescriptorSet,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &bufferInfos.back()
    });

    return *this;
}

DescriptorSetWriter& DescriptorSetWriter::WriteImage(const uint32_t binding,
                                                     const vk::ImageView imageView,
                                                     const vk::Sampler sampler,
                                                     const vk::ImageLayout layout,
                                                     const vk::DescriptorType type) {
    imageInfos.push_back({
        .sampler = sampler,
        .imageView = imageView,
        .imageLayout = layout
    });

    writes.push_back({
        .sType = vk::StructureType::eWriteDescriptorSet,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &imageInfos.back()
    });

    return *this;
}

DescriptorSetWriter& DescriptorSetWriter::WriteStorageImage(const uint32_t binding,
                                                            const vk::ImageView imageView,
                                                            const vk::ImageLayout layout) {
    imageInfos.push_back({
        .imageView = imageView,
        .imageLayout = layout
    });

    writes.push_back({
        .sType = vk::StructureType::eWriteDescriptorSet,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageImage,
        .pImageInfo = &imageInfos.back()
    });

    return *this;
}

DescriptorSetWriter& DescriptorSetWriter::WriteCombinedImageSampler(const uint32_t binding,
                                                                    const vk::Sampler sampler,
                                                                    const vk::ImageView imageView,
                                                                    const vk::ImageLayout layout) {
    imageInfos.push_back({
        .sampler = sampler,
        .imageView = imageView,
        .imageLayout = layout
    });

    writes.push_back({
        .sType = vk::StructureType::eWriteDescriptorSet,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo = &imageInfos.back()
    });

    return *this;
}

void DescriptorSetWriter::Update(const vk::Device device, const vk::DescriptorSet descriptorSet) {
    for (auto& write : writes) write.dstSet = descriptorSet;
    device.updateDescriptorSets(writes, {});
}

void DescriptorSetWriter::Clear() {
    writes.clear();
    bufferInfos.clear();
    imageInfos.clear();
}
