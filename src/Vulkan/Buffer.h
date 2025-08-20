#pragma once

#include "Vulkan/Base.h"
#include "VulkanContext.h"

class Buffer {
public:
    Buffer(const std::shared_ptr<VulkanContext>& context,
           vk::DeviceSize size,
           vk::BufferUsageFlags usage,
           vk::MemoryPropertyFlags properties =
               vk::MemoryPropertyFlagBits::eHostVisible |
               vk::MemoryPropertyFlagBits::eHostCoherent);

    template <typename T>
    Buffer(const std::shared_ptr<VulkanContext>& context,
           const std::vector<T>& data,
           const vk::BufferUsageFlags usage,
           const vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible |
               vk::MemoryPropertyFlagBits::eHostCoherent)
        : Buffer(context, sizeof(T) * data.size(), usage, properties) {
        Update(data);
    }

    template <typename T>
    Buffer(const std::shared_ptr<VulkanContext>& context,
           const T& data,
           const vk::BufferUsageFlags usage,
           const vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible |
               vk::MemoryPropertyFlagBits::eHostCoherent)
        : Buffer(context, sizeof(T), usage, properties) {
        Update(data);
    }

    ~Buffer();

    template <typename T>
    void Update(const std::vector<T>& data) const {
        Update(data.data(), sizeof(T) * data.size());
    }

    template <typename T>
    void Update(const T& data) const {
        Update(&data, sizeof(T));
    }

    vk::Buffer GetHandle() const { return buffer; }
    vk::DeviceMemory GetMemory() const { return memory; }
    vk::DeviceSize GetSize() const { return bufferSize; }

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

private:
    void Update(const void* data, vk::DeviceSize size) const;

private:
    std::shared_ptr<VulkanContext> vulkanContext;

    vk::Buffer buffer;
    vk::DeviceSize bufferSize;
    vk::DeviceMemory memory;
};

class BufferWithStaging {
public:
    BufferWithStaging(const std::shared_ptr<VulkanContext>& context, vk::DeviceSize size, vk::BufferUsageFlags usage);

    void Upload(vk::CommandBuffer commandBuffer,
                vk::PipelineStageFlags2 dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                vk::AccessFlags2 dstAccessMask = vk::AccessFlagBits2::eShaderRead) const;

    template <typename T>
    void Update(const std::vector<T>& data) const {
        stagingBuffer->Update(data);
        stagedSize = sizeof(T) * data.size();
        needsUpload = true;
    }

    template <typename T>
    void Update(const T& data) const {
        stagingBuffer->Update(data);
        stagedSize = sizeof(T);
        needsUpload = true;
    }

    bool ShouldUpload() const { return needsUpload; }
    vk::Buffer GetHandle() const { return buffer->GetHandle(); }
    vk::DeviceSize GetSize() const { return buffer->GetSize(); }

private:
    std::unique_ptr<Buffer> buffer;
    std::unique_ptr<Buffer> stagingBuffer;

    mutable vk::DeviceSize stagedSize = 0;
    mutable bool needsUpload = false;
};
