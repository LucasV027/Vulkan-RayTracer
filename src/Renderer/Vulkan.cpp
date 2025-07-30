#include "Vulkan.h"

#include "Utils.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "Log.h"

#ifndef NDEBUG
vk::Bool32 vkHelpers::DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                    vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
                                    vk::DebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                    void* pUserData) {
    if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
        LOGE("{} Validation Layer: Error: {}: {}\n",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        LOGW("{} Validation Layer: Warning: {}: {}\n",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
        LOGI("{} Validation Layer: Information: {}: {}\n",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    } else if (messageTypes & vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance) {
        LOGI("{} Validation Layer: Performance warning: {}: {}\n",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose) {
        LOGD("{} Validation Layer: Verbose: {}: {}\n",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    }
    return false;
}
#endif

vk::UniqueShaderModule vkHelpers::CreateShaderModule(const vk::Device device, const std::filesystem::path& filepath) {
    auto code = Utils::ReadSpirvFile(filepath);
    if (!code) {
        throw std::runtime_error(std::format("{}", code.error()));
    }

    const vk::ShaderModuleCreateInfo createModuleInfo{
        .codeSize = code.value().size() * sizeof(uint32_t),
        .pCode = code.value().data()
    };

    return device.createShaderModuleUnique(createModuleInfo);
}

void vkHelpers::TransitionImageLayout(const vk::CommandBuffer cmd,
                                      const vk::Image image,
                                      const vk::ImageLayout oldLayout,
                                      const vk::ImageLayout newLayout,
                                      const vk::AccessFlags2 srcAccessMask,
                                      const vk::AccessFlags2 dstAccessMask,
                                      const vk::PipelineStageFlags2 srcStage,
                                      const vk::PipelineStageFlags2 dstStage) {
    // Initialize the VkImageMemoryBarrier2 structure
    vk::ImageMemoryBarrier2 imageBarrier{
        // Specify the pipeline stages and access masks for the barrier
        .srcStageMask = srcStage,       // Source pipeline stage mask
        .srcAccessMask = srcAccessMask, // Source access mask
        .dstStageMask = dstStage,       // Destination pipeline stage mask
        .dstAccessMask = dstAccessMask, // Destination access mask

        // Specify the old and new layouts of the image
        .oldLayout = oldLayout, // Current layout of the image
        .newLayout = newLayout, // Target layout of the image

        // We are not changing the ownership between queues
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,

        // Specify the image to be affected by this barrier
        .image = image,

        // Define the subresource range (which parts of the image are affected)
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor, // Affects the color aspect of the image
            .baseMipLevel = 0,                             // Start at mip level 0
            .levelCount = 1,                               // Number of mip levels affected
            .baseArrayLayer = 0,                           // Start at array layer 0
            .layerCount = 1                                // Number of array layers affected
        }
    };

    // Initialize the VkDependencyInfo structure
    const vk::DependencyInfo dependencyInfo{
        .dependencyFlags = {},                // No special dependency flags
        .imageMemoryBarrierCount = 1,         // Number of image memory barriers
        .pImageMemoryBarriers = &imageBarrier // Pointer to the image memory barrier(s)
    };

    // Record the pipeline barrier into the command buffer
    cmd.pipelineBarrier2(dependencyInfo);
}

uint32_t vkHelpers::FindMemoryType(const vk::PhysicalDevice physicalDevice,
                                   const uint32_t typeFilter,
                                   const vk::MemoryPropertyFlags properties) {
    const auto memProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("Failed to find suitable memory type.");
}

void vkHelpers::AllocatedBuffer::Destroy(const vk::Device device) {
    if (buffer) device.destroyBuffer(buffer);
    if (memory) device.freeMemory(memory);
    buffer = nullptr;
    memory = nullptr;
}

vkHelpers::AllocatedBuffer vkHelpers::CreateBuffer(const vk::Device device,
                                                   const vk::PhysicalDevice physicalDevice,
                                                   const vk::DeviceSize size,
                                                   const vk::BufferUsageFlags usage,
                                                   const vk::MemoryPropertyFlags properties) {
    const vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    const vk::Buffer buffer = device.createBuffer(bufferInfo);

    const vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

    const vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties),
    };

    const vk::DeviceMemory memory = device.allocateMemory(allocInfo);
    device.bindBufferMemory(buffer, memory, 0);

    return AllocatedBuffer{buffer, memory};
}

void vkHelpers::AllocatedImage::Destroy(const vk::Device device) {
    if (image) device.destroyImage(image);
    if (view) device.destroyImageView(view);
    if (memory) device.freeMemory(memory);
    image = nullptr;
    memory = nullptr;
    view = nullptr;
}

vkHelpers::AllocatedImage vkHelpers::CreateStorageImage(const vk::Device device,
                                                        const vk::PhysicalDevice physicalDevice,
                                                        const uint32_t width,
                                                        const uint32_t height,
                                                        const vk::Format format) {
    constexpr vk::ImageUsageFlags usage =
        vk::ImageUsageFlagBits::eStorage |
        vk::ImageUsageFlagBits::eSampled |
        vk::ImageUsageFlagBits::eTransferSrc;

    const vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = vk::Extent3D{width, height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    const vk::Image image = device.createImage(imageInfo);
    const vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);

    const vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits,
                                          vk::MemoryPropertyFlagBits::eDeviceLocal),
    };

    const vk::DeviceMemory memory = device.allocateMemory(allocInfo);
    device.bindImageMemory(image, memory, 0);

    const vk::ImageViewCreateInfo viewInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    const vk::ImageView view = device.createImageView(viewInfo);

    return AllocatedImage{image, memory, view};
}
