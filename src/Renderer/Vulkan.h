#pragma once

#include <filesystem>
#include <cstring> // memcpy

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "Log.h"

namespace vkHelpers {
#ifndef NDEBUG
    vk::Bool32 DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                             vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
                             vk::DebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                             void* pUserData);
#endif

    vk::UniqueShaderModule CreateShaderModule(vk::Device device, const std::filesystem::path& filepath);

    void TransitionImageLayout(vk::CommandBuffer cmd,
                               vk::Image image,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               vk::AccessFlags2 srcAccessMask,
                               vk::AccessFlags2 dstAccessMask,
                               vk::PipelineStageFlags2 srcStage,
                               vk::PipelineStageFlags2 dstStage);

    uint32_t FindMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    struct AllocatedImage {
        vk::Image image = nullptr;
        vk::DeviceMemory memory = nullptr;
        vk::ImageView view = nullptr;

        void Destroy(vk::Device device);
    };

    AllocatedImage CreateStorageImage(vk::Device device,
                                      vk::PhysicalDevice physicalDevice,
                                      uint32_t width,
                                      uint32_t height,
                                      vk::Format format);
}
