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
        LOGE("{}", code.error());
    }

    const vk::ShaderModuleCreateInfo createModuleInfo{
        .codeSize = code.value().size() * sizeof(uint32_t),
        .pCode = code.value().data()
    };

    return device.createShaderModule(createModuleInfo);
}



