#pragma once

#include <filesystem>

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace vkHelpers {
#ifndef NDEBUG
    vk::Bool32 DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                             vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
                             vk::DebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                             void* pUserData);
#endif

    vk::ShaderModule CreateShaderModule(vk::Device device, const std::filesystem::path& filepath);

    void TransitionImageLayout(vk::CommandBuffer cmd,
                               vk::Image image,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               vk::AccessFlags2 srcAccessMask,
                               vk::AccessFlags2 dstAccessMask,
                               vk::PipelineStageFlags2 srcStage,
                               vk::PipelineStageFlags2 dstStage);
}
