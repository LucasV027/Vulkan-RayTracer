#pragma once

#include "Vulkan.h"
#include "VulkanContext.h"

class Image {
public:
    Image(const std::shared_ptr<VulkanContext>& context,
          uint32_t width,
          uint32_t height,
          vk::Format format,
          vk::ImageUsageFlags usage,
          vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
          uint32_t mipLevels = 1);

    ~Image();

    vk::UniqueImageView CreateView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor,
                                   uint32_t mipLevel = 0,
                                   uint32_t levelCount = 1) const;

    void TransitionLayout(vk::CommandBuffer cmd,
                          vk::ImageLayout oldLayout,
                          vk::ImageLayout newLayout,
                          vk::PipelineStageFlags2 srcStage,
                          vk::PipelineStageFlags2 dstStage) const;

    vk::Image GetHandle() const { return image; }
    vk::DeviceMemory GetMemory() const { return memory; }
    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }
    vk::Format GetFormat() const { return format; }

    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

private:
    std::shared_ptr<VulkanContext> vulkanContext;

    vk::Image image;
    vk::DeviceMemory memory;
    uint32_t width, height;
    vk::Format format;
    uint32_t mipLevels;
};
