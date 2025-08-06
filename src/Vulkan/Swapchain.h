#pragma once

#include "Vulkan/Base.h"
#include "VulkanContext.h"
#include "Window.h"

struct FrameContext {
    vk::CommandPool commandPool = nullptr;
    vk::CommandBuffer commandBuffer = nullptr;
    vk::UniqueSemaphore imageAvailable;
    vk::Semaphore renderFinished = nullptr;
    vk::Fence inFlight = nullptr;
    uint32_t index;
};

class Swapchain {
public:
    Swapchain(const std::shared_ptr<VulkanContext>& context, const std::shared_ptr<Window>& window);
    ~Swapchain();

    void Recreate();

    vk::SwapchainKHR GetSwapchain() const;
    vk::Extent2D GetExtent() const;
    vk::Format GetFormat() const;
    uint32_t GetImageCount() const;
    const std::vector<vk::ImageView>& GetImageViews() const;
    const std::vector<vk::Image>& GetImages() const;

    FrameContext& GetCurrentFrameContext();

    void SetCurrentImageIndex(uint32_t index);
    void ResetCurrentImageIndex();

private:
    void CreateSwapchain();
    void CreateImageViews();
    void CreateFrameContexts();

    void DestroyImageViews();
    void DestroyFrameContexts();

private:
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<Window> window;

    std::optional<uint32_t> currentImageIndex;

    vk::SwapchainKHR swapchain = nullptr;
    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;
    std::vector<FrameContext> frameContexts;

    vk::Format format = vk::Format::eUndefined;
    vk::Extent2D extent{};
};
