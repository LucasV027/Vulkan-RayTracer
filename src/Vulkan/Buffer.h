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

class StorageBuffer {
public:
    StorageBuffer(const std::shared_ptr<VulkanContext>& context, vk::DeviceSize initialSize);

    void Upload(vk::CommandBuffer commandBuffer,
                vk::PipelineStageFlags2 dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                vk::AccessFlags2 dstAccessMask = vk::AccessFlagBits2::eShaderRead) const;

    template <typename T>
    void Update(const std::vector<T>& data) {
        const vk::DeviceSize copySize = sizeof(T) * data.size();
        EnsureCapacity(copySize);

        stagingBuffer->Update(data);
        stagedSize = copySize;

        needsUpload = true;
    }

    template <typename T>
    void Update(const T& data) {
        const vk::DeviceSize copySize = sizeof(T);
        EnsureCapacity(copySize);

        stagingBuffer->Update(data);
        stagedSize = copySize;
        needsUpload = true;
    }

    bool ShouldUpload() const { return needsUpload; }
    bool Changed() const { return changed; }
    void ResetChanged() const { changed = false; }
    vk::Buffer GetHandle() const { return buffer->GetHandle(); }
    vk::DeviceSize GetSize() const { return buffer->GetSize(); }

private:
    void EnsureCapacity(vk::DeviceSize requiredSize);
    void CreateBuffers(vk::DeviceSize size);

private:
    static constexpr float GROW_FACTOR = 2.0f;

    std::shared_ptr<VulkanContext> context;

    std::unique_ptr<Buffer> buffer;
    std::unique_ptr<Buffer> stagingBuffer;

    mutable vk::DeviceSize stagedSize = 0;
    mutable bool needsUpload = false;
    mutable bool changed = false;
};
