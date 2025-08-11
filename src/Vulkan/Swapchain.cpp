#include "Swapchain.h"

Swapchain::Swapchain(const std::shared_ptr<VulkanContext>& context, const std::shared_ptr<Window>& window) :
    vulkanContext(context),
    window(window) {
    CreateSwapchain();
}

Swapchain::~Swapchain() {
    DestroyImageViews();
    DestroyFrameContexts();

    vulkanContext->device.destroySwapchainKHR(swapchain);
}

void Swapchain::Recreate() {
    CreateSwapchain();
}

vk::SwapchainKHR Swapchain::GetSwapchain() const { return swapchain; }
vk::Extent2D Swapchain::GetExtent() const { return extent; }
vk::Format Swapchain::GetFormat() const { return format; }
uint32_t Swapchain::GetImageCount() const { return images.size(); }
const std::vector<vk::ImageView>& Swapchain::GetImageViews() const { return imageViews; }
const std::vector<vk::Image>& Swapchain::GetImages() const { return images; }

FrameContext& Swapchain::GetCurrentFrameContext() {
    if (!currentImageIndex.has_value()) {
        throw std::runtime_error("Swapchain::GetCurrentFrameContext() : Current image index not set.");
    }
    frameContexts[*currentImageIndex].index = *currentImageIndex;
    return frameContexts[*currentImageIndex];
}

void Swapchain::SetCurrentImageIndex(uint32_t index) { currentImageIndex = index; }
void Swapchain::ResetCurrentImageIndex() { currentImageIndex.reset(); }

void Swapchain::CreateSwapchain() {
    const vk::SurfaceCapabilitiesKHR capabilities = vulkanContext->physicalDevice.getSurfaceCapabilitiesKHR(
        vulkanContext->surface);
    auto formats = vulkanContext->physicalDevice.getSurfaceFormatsKHR(vulkanContext->surface);
    auto presentModes = vulkanContext->physicalDevice.getSurfacePresentModesKHR(vulkanContext->surface);

    vk::SurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            surfaceFormat = availableFormat;
            break;
        }
    }
    format = surfaceFormat.format;

    auto presentMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            presentMode = availablePresentMode;
            break;
        }
    }

    const auto width = window->GetWidth();
    const auto height = window->GetHeight();
    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    } else {
        extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    uint32_t imageCount = 3;
    if (capabilities.maxImageCount > 0)
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    imageCount = std::max(imageCount, capabilities.minImageCount);

    const auto oldSwapchain = swapchain;

    const vk::SwapchainCreateInfoKHR createInfo{
        .surface = vulkanContext->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchain
    };

    swapchain = vulkanContext->device.createSwapchainKHR(createInfo);
    images = vulkanContext->device.getSwapchainImagesKHR(swapchain);

    if (oldSwapchain) {
        vulkanContext->device.waitIdle();

        DestroyImageViews();
        DestroyFrameContexts();
        vulkanContext->device.destroySwapchainKHR(oldSwapchain);
    }

    CreateImageViews();
    CreateFrameContexts();
}

void Swapchain::CreateImageViews() {
    assert(imageViews.empty());

    for (const auto& image : images) {
        vk::ImageViewCreateInfo viewCreateInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = format,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        imageViews.push_back(vulkanContext->device.createImageView(viewCreateInfo));
    }
}

void Swapchain::CreateFrameContexts() {
    assert(frameContexts.empty());

    frameContexts.resize(images.size());

    for (auto& fc : frameContexts) {
        const vk::CommandPoolCreateInfo commandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = vulkanContext->graphicsQueueIndex,
        };

        fc.commandPool = vulkanContext->device.createCommandPool(commandPoolCreateInfo);

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
            .commandPool = fc.commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        fc.commandBuffer = vulkanContext->device.allocateCommandBuffers(commandBufferAllocateInfo).front();

        fc.inFlight = vulkanContext->device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
        fc.renderFinished = vulkanContext->device.createSemaphore({});
    }
}

void Swapchain::DestroyImageViews() {
    if (!imageViews.empty()) {
        for (const auto& imageView : imageViews) vulkanContext->device.destroyImageView(imageView);
        imageViews.clear();
    }
}

void Swapchain::DestroyFrameContexts() {
    if (!frameContexts.empty()) {
        for (auto& fc : frameContexts) {
            if (fc.inFlight) vulkanContext->device.destroyFence(fc.inFlight);
            if (fc.commandBuffer) vulkanContext->device.freeCommandBuffers(fc.commandPool, fc.commandBuffer);
            if (fc.commandPool) vulkanContext->device.destroyCommandPool(fc.commandPool);
            if (fc.renderFinished) vulkanContext->device.destroySemaphore(fc.renderFinished);
        }
        frameContexts.clear();
    }
}
