#include "Buffer.h"

#include "Core/Log.h"

Buffer::Buffer(const std::shared_ptr<VulkanContext>& context,
               const vk::DeviceSize size,
               const vk::BufferUsageFlags usage,
               const vk::MemoryPropertyFlags properties) : vulkanContext(context), bufferSize(size) {
    const vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    buffer = context->device.createBuffer(bufferInfo);

    const vk::MemoryRequirements memRequirements = context->device.getBufferMemoryRequirements(buffer);

    const vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vkHelpers::FindMemoryType(context->physicalDevice, memRequirements.memoryTypeBits,
                                                     properties),
    };

    memory = context->device.allocateMemory(allocInfo);
    context->device.bindBufferMemory(buffer, memory, 0);
}

Buffer::~Buffer() {
    if (buffer) vulkanContext->device.destroyBuffer(buffer);
    if (memory) vulkanContext->device.freeMemory(memory);
}

Buffer::Buffer(Buffer&& other) noexcept : vulkanContext(std::move(other.vulkanContext)),
                                          buffer(std::exchange(other.buffer, vk::Buffer{})),
                                          bufferSize(other.bufferSize),
                                          memory(std::exchange(other.memory, vk::DeviceMemory{})) {}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        if (buffer) vulkanContext->device.destroyBuffer(buffer);
        if (memory) vulkanContext->device.freeMemory(memory);

        vulkanContext = std::move(other.vulkanContext);
        buffer = std::exchange(other.buffer, vk::Buffer{});
        memory = std::exchange(other.memory, vk::DeviceMemory{});
        bufferSize = other.bufferSize;
    }
    return *this;
}

void Buffer::Update(const void* data, const vk::DeviceSize size) const {
    if (size > bufferSize) {
        LOGE("Trying to update {} bytes in buffer of size {}", size, bufferSize);
        return;
    }

    vk::DeviceSize copySize = size;
    const void* srcData = data;

    // size == 0 means clear the buffer
    if (size == 0) copySize = bufferSize;

    void* mapped = nullptr;
    const vk::Result result = vulkanContext->device.mapMemory(memory, 0, copySize, {}, &mapped);

    if (result == vk::Result::eSuccess) {
        if (size == 0) {
            memset(mapped, 0, copySize);
        } else {
            memcpy(mapped, srcData, copySize);
        }
        vulkanContext->device.unmapMemory(memory);
    } else {
        LOGE("Failed to map memory! Error: {}", vk::to_string(result));
    }
}

StorageBuffer::StorageBuffer(const std::shared_ptr<VulkanContext>& context, const vk::DeviceSize initialSize) :
    context(context) {
    // Buffer device-local (GPU)
    buffer = std::make_unique<Buffer>(
        context,
        initialSize,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    // Buffer staging (CPU visible)
    stagingBuffer = std::make_unique<Buffer>(
        context,
        initialSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );

    needsUpload = false;
}

void StorageBuffer::Upload(const vk::CommandBuffer commandBuffer,
                           const vk::PipelineStageFlags2 dstStageMask,
                           const vk::AccessFlags2 dstAccessMask) const {
    if (!needsUpload || stagedSize == 0) {
        return;
    }

    const vk::BufferCopy copyRegion{
        .size = stagedSize,
    };

    commandBuffer.copyBuffer(stagingBuffer->GetHandle(), buffer->GetHandle(), copyRegion);

    const vk::BufferMemoryBarrier2 barrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .buffer = buffer->GetHandle(),
        .offset = 0,
        .size = stagedSize,
    };

    const vk::DependencyInfo depInfo{
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &barrier
    };
    commandBuffer.pipelineBarrier2(depInfo);

    needsUpload = false;
}

void StorageBuffer::EnsureCapacity(const vk::DeviceSize requiredSize) {
    if (requiredSize <= buffer->GetSize()) return;

    auto growingSize = buffer->GetSize();
    while (growingSize < requiredSize) {
        growingSize *= GROW_FACTOR;
    }

    context->device.waitIdle();
    changed = true;
    CreateBuffers(growingSize);
}

void StorageBuffer::CreateBuffers(vk::DeviceSize size) {
    buffer = std::make_unique<Buffer>(
        context,
        size,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    stagingBuffer = std::make_unique<Buffer>(
        context,
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
}
