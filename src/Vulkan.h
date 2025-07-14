#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "Log.h"

namespace vkd {
#ifndef NDEBUG
    inline vk::Bool32 DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
}
