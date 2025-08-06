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

    void* mapped;
    const vk::Result result = vulkanContext->device.mapMemory(memory, 0, size, {}, &mapped);

    if (result == vk::Result::eSuccess) {
        memcpy(mapped, data, size);
        vulkanContext->device.unmapMemory(memory);
    } else {
        LOGE("Failed to map memory! Error: {}", vk::to_string(result));
    }
}
