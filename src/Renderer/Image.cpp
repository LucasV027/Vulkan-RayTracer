#include "Image.h"

Image::Image(const std::shared_ptr<VulkanContext>& context,
             const uint32_t width,
             const uint32_t height,
             const vk::Format format,
             const vk::ImageUsageFlags usage,
             const vk::MemoryPropertyFlags properties,
             const vk::ImageTiling tiling,
             const uint32_t mipLevels)
    : context(context),
      width(width),
      height(height),
      format(format),
      mipLevels(mipLevels) {
    const vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    image = context->device.createImage(imageInfo);

    const vk::MemoryRequirements memRequirements = context->device.getImageMemoryRequirements(image);

    const vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vkHelpers::FindMemoryType(context->physicalDevice,
                                                     memRequirements.memoryTypeBits,
                                                     properties)
    };

    memory = context->device.allocateMemory(allocInfo);
    context->device.bindImageMemory(image, memory, 0);
}

Image::~Image() {
    if (image) context->device.destroyImage(image);
    if (memory) context->device.freeMemory(memory);
}

vk::ImageView Image::CreateView(const vk::ImageAspectFlags aspectFlags,
                                const uint32_t mipLevel,
                                const uint32_t levelCount) const {
    const vk::ImageViewCreateInfo viewInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = mipLevel,
            .levelCount = levelCount,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    return context->device.createImageView(viewInfo);
}

void Image::TransitionLayout(const vk::CommandBuffer cmd,
                             const vk::ImageLayout oldLayout,
                             const vk::ImageLayout newLayout,
                             const vk::PipelineStageFlags2 srcStage,
                             const vk::PipelineStageFlags2 dstStage) const {
    vk::AccessFlags2 srcAccess, dstAccess;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eGeneral) {
        srcAccess = {};
        dstAccess = vk::AccessFlagBits2::eShaderWrite;
    } else if (oldLayout == vk::ImageLayout::eGeneral && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        srcAccess = vk::AccessFlagBits2::eShaderWrite;
        dstAccess = vk::AccessFlagBits2::eShaderRead;
    }

    const vk::ImageMemoryBarrier2 barrier{
        .srcStageMask = srcStage,
        .srcAccessMask = srcAccess,
        .dstStageMask = dstStage,
        .dstAccessMask = dstAccess,

        .oldLayout = oldLayout,
        .newLayout = newLayout,

        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,

        .image = image,

        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    const vk::DependencyInfo dependencyInfo{
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    cmd.pipelineBarrier2(dependencyInfo);
}

Image::Image(Image&& other) noexcept : context(std::move(other.context)),
                                       image(std::exchange(other.image, vk::Image{})),
                                       memory(std::exchange(other.memory, vk::DeviceMemory{})),
                                       width(other.width),
                                       height(other.height),
                                       format(other.format),
                                       mipLevels(other.mipLevels) {}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        if (image) context->device.destroyImage(image);
        if (memory) context->device.freeMemory(memory);

        context = std::move(other.context);
        image = std::exchange(other.image, vk::Image{});
        memory = std::exchange(other.memory, vk::DeviceMemory{});
        width = other.width;
        height = other.height;
        format = other.format;
        mipLevels = other.mipLevels;
    }
    return *this;
}
