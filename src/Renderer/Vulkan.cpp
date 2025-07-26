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
        LOGE("{} Validation Layer: Error: {}: {}",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        LOGW("{} Validation Layer: Warning: {}: {}",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
        LOGI("{} Validation Layer: Information: {}: {}",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    } else if (messageTypes & vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance) {
        LOGI("{} Validation Layer: Performance warning: {}: {}",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose) {
        LOGD("{} Validation Layer: Verbose: {}: {}",
             pCallbackData->messageIdNumber,
             pCallbackData->pMessageIdName,
             pCallbackData->pMessage);
    }
    return false;
}
#endif

vk::ShaderModule vkHelpers::CreateShaderModule(const vk::Device device, const std::filesystem::path& filepath) {
    auto code = Utils::ReadSpirvFile(filepath);
    if (!code) {
        throw std::runtime_error(std::format("{}", code.error()));
    }

    const vk::ShaderModuleCreateInfo createModuleInfo{
        .codeSize = code.value().size() * sizeof(uint32_t),
        .pCode = code.value().data()
    };

    return device.createShaderModule(createModuleInfo);
}

void vkHelpers::TransitionImageLayout(vk::CommandBuffer cmd,
                                      vk::Image image,
                                      vk::ImageLayout oldLayout,
                                      vk::ImageLayout newLayout,
                                      vk::AccessFlags2 srcAccessMask,
                                      vk::AccessFlags2 dstAccessMask,
                                      vk::PipelineStageFlags2 srcStage,
                                      vk::PipelineStageFlags2 dstStage) {
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
    vk::DependencyInfo dependencyInfo{
        .dependencyFlags = {},                // No special dependency flags
        .imageMemoryBarrierCount = 1,         // Number of image memory barriers
        .pImageMemoryBarriers = &imageBarrier // Pointer to the image memory barrier(s)
    };

    // Record the pipeline barrier into the command buffer
    cmd.pipelineBarrier2(dependencyInfo);
}



