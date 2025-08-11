#pragma once

#include "Window/Window.h"
#include "Vulkan/Base.h"

class VulkanContext {
public:
    explicit VulkanContext(const std::shared_ptr<Window>& window);
    ~VulkanContext();

public:
    vk::Instance instance = nullptr;
    vk::DebugUtilsMessengerEXT debugCallback = nullptr;
    vk::SurfaceKHR surface = nullptr;

    vk::PhysicalDevice physicalDevice = nullptr;
    vk::Device device = nullptr;

    uint32_t graphicsQueueIndex = -1;
    vk::Queue graphicsQueue = nullptr;

    vk::DescriptorPool mainDescriptorPool = nullptr;
    vk::CommandPool commandPool = nullptr;

public:
    vk::CommandBuffer BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(vk::CommandBuffer commandBuffer) const;

private:
    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateDescriptorPool();
    void CreateCommandPool();

private:
    std::shared_ptr<Window> windowRef;
};
