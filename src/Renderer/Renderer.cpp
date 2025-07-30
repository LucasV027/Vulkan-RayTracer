#include "Renderer.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <set>

#include "Log.h"

Renderer::Renderer(const std::shared_ptr<VulkanContext>& context,
                   const std::shared_ptr<Window>& window) : context(context), window(window) {
    CreateSwapChain();

    computePipeline = std::make_unique<ComputePipeline>(context);
    graphicsPipeline = std::make_unique<GraphicsPipeline>(context, swapChainDimensions.format);
    uiPipeline = std::make_unique<ImGuiPipeline>(context, window, swapChainDimensions.format, swapChainImages.size());
}

Renderer::~Renderer() {
    // Don't release anything until the GPU is completely idle.
    if (context->device) context->device.waitIdle();

    Cleanup();
}

void Renderer::Draw() {
    if (const auto fc = BeginFrame()) {
        computePipeline->UpdateUniform(14); // Test
        computePipeline->Dispatch(fc->commandBuffer, 16, 1, 1);

        graphicsPipeline->Render(fc->commandBuffer, swapChainImagesViews[fc->index], swapChainDimensions.width,
                                 swapChainDimensions.height);

        uiPipeline->Render(fc->commandBuffer, swapChainImagesViews[fc->index], swapChainDimensions.width,
                           swapChainDimensions.height);

        Submit(*fc);
        Present(*fc);
    }
}

void Renderer::Begin() const {
    uiPipeline->Begin();
}

FrameContext* Renderer::BeginFrame() {
    const auto acquireResult = AcquireNextImage();
    if (!acquireResult) {
        if (acquireResult.error() == AcquireError::Failed) context->device.waitIdle();
        else Resize();
        return nullptr;
    }

    const uint32_t index = *acquireResult;
    auto& frame = perFrames[index];
    frame.index = index;

    context->device.resetCommandPool(frame.commandPool);

    frame.commandBuffer.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    });

    vkHelpers::TransitionImageLayout(frame.commandBuffer,
                                     swapChainImages[frame.index],
                                     vk::ImageLayout::eUndefined,
                                     vk::ImageLayout::eColorAttachmentOptimal,
                                     {}, // srcAccessMask (no need to wait for previous operations)
                                     vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
                                     vk::PipelineStageFlagBits2::eTopOfPipe, // srcStage
                                     vk::PipelineStageFlagBits2::eColorAttachmentOutput // dstStage
    );

    return &frame;
}

void Renderer::Submit(const FrameContext& fc) const {
    vkHelpers::TransitionImageLayout(fc.commandBuffer,
                                     swapChainImages[fc.index],
                                     vk::ImageLayout::eColorAttachmentOptimal,
                                     vk::ImageLayout::ePresentSrcKHR,
                                     vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
                                     {},                                                 // dstAccessMask
                                     vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                                     vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
    );

    fc.commandBuffer.end();

    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &fc.imageAvailable.get(),
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &fc.commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &fc.renderFinished
    };

    context->graphicsQueue.submit(submitInfo, fc.inFlight);
}

void Renderer::Present(const FrameContext& fc) {
    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &fc.renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &swapChain,
        .pImageIndices = &fc.index
    };

    try {
        const auto result = context->graphicsQueue.presentKHR(presentInfo);
        if (result == vk::Result::eSuboptimalKHR) {
            Resize();
        }
    } catch (vk::OutOfDateKHRError&) {
        Resize();
    }
}

void Renderer::Cleanup() {
    for (auto& perFrame : perFrames) perFrame.Destroy(context->device);
    perFrames.clear();

    for (const vk::ImageView imageView : swapChainImagesViews) context->device.destroyImageView(imageView);

    if (swapChain) context->device.destroySwapchainKHR(swapChain);
}

std::expected<uint32_t, Renderer::AcquireError> Renderer::AcquireNextImage() {
    auto acquireSemaphore = context->device.createSemaphoreUnique({});

    uint32_t imageIndex;
    vk::Result result;
    try {
        std::tie(result, imageIndex) = context->device.acquireNextImageKHR(
            swapChain, UINT64_MAX, acquireSemaphore.get());
    } catch (vk::OutOfDateKHRError&) {
        return std::unexpected(AcquireError::OutOfDate);
    }

    if (result == vk::Result::eSuboptimalKHR) return std::unexpected(AcquireError::Suboptimal);
    if (result != vk::Result::eSuccess) return std::unexpected(AcquireError::Failed);

    if (perFrames[imageIndex].inFlight) {
        const auto waitResult = context->device.waitForFences(perFrames[imageIndex].inFlight, true, UINT64_MAX);
        assert(waitResult == vk::Result::eSuccess);
        context->device.resetFences(perFrames[imageIndex].inFlight);
    }

    perFrames[imageIndex].imageAvailable = std::move(acquireSemaphore);

    return imageIndex;
}

bool Renderer::PresentImage(uint32_t swapChainIndex) {
    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &perFrames[swapChainIndex].renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &swapChain,
        .pImageIndices = &swapChainIndex
    };

    try {
        const auto result = context->graphicsQueue.presentKHR(presentInfo);
        if (result == vk::Result::eSuboptimalKHR) {
            return false;
        }
    } catch (vk::OutOfDateKHRError&) {
        return false;
    }

    return true;
}

void Renderer::Resize() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        std::tie(width, height) = window->GetFrameBufferSize();
        glfwWaitEvents();
    }

    context->device.waitIdle();

    const auto surfaceProperties = context->physicalDevice.getSurfaceCapabilitiesKHR(context->surface);

    const bool dimensionsChanged =
        surfaceProperties.currentExtent.width != swapChainDimensions.width ||
        surfaceProperties.currentExtent.height != swapChainDimensions.height;

    if (dimensionsChanged) {
        CreateSwapChain();
    }
}

void Renderer::CreateSwapChain() {
    vk::SurfaceCapabilitiesKHR capabilities = context->physicalDevice.getSurfaceCapabilitiesKHR(context->surface);
    auto formats = context->physicalDevice.getSurfaceFormatsKHR(context->surface);
    auto presentModes = context->physicalDevice.getSurfacePresentModesKHR(context->surface);

    vk::SurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    swapChainDimensions.format = surfaceFormat.format;

    auto presentMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            presentMode = availablePresentMode;
            break;
        }
    }

    auto [width, height] = window->GetSize();
    swapChainDimensions.width = width;
    swapChainDimensions.height = height;

    vk::Extent2D extent;
    if (capabilities.currentExtent.width == UINT32_MAX) {
        extent.width = std::clamp(swapChainDimensions.width,
                                  capabilities.minImageExtent.width,
                                  capabilities.maxImageExtent.width);
        extent.height = std::clamp(swapChainDimensions.height,
                                   capabilities.minImageExtent.height,
                                   capabilities.maxImageExtent.height);
    } else {
        extent = capabilities.currentExtent;
    }


    uint32_t imageCount = 3;
    if (capabilities.maxImageCount > 0)
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    imageCount = std::max(imageCount, capabilities.minImageCount);

    const auto oldSwapChain = swapChain;

    const vk::SwapchainCreateInfoKHR createInfo{
        .surface = context->surface,
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
        .clipped = true,
        .oldSwapchain = oldSwapChain
    };

    swapChain = context->device.createSwapchainKHR(createInfo);
    swapChainImages = context->device.getSwapchainImagesKHR(swapChain);

    if (oldSwapChain) {
        for (vk::ImageView imageView : swapChainImagesViews) context->device.destroyImageView(imageView);
        swapChainImagesViews.clear();

        for (auto& perFrame : perFrames) perFrame.Destroy(context->device);

        context->device.destroySwapchainKHR(oldSwapChain);
    }

    for (auto const& swapChainImage : swapChainImages) {
        vk::ImageViewCreateInfo viewCreateInfo{
            .image = swapChainImage,
            .viewType = vk::ImageViewType::e2D,
            .format = swapChainDimensions.format,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        swapChainImagesViews.push_back(context->device.createImageView(viewCreateInfo));
    }

    perFrames.clear();
    perFrames.resize(swapChainImages.size());

    for (auto& perFrame : perFrames) {
        perFrame.Init(context->device, context->graphicsQueueIndex);
    }
}

void FrameContext::Init(const vk::Device device, const uint32_t graphicsQueueIndex) {
    vk::CommandPoolCreateInfo commandPoolCreateInfo{
        .flags = vk::CommandPoolCreateFlagBits::eTransient,
        .queueFamilyIndex = graphicsQueueIndex,
    };

    commandPool = device.createCommandPool(commandPoolCreateInfo);

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo)[0];

    inFlight = device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
    imageAvailable = device.createSemaphoreUnique({});
    renderFinished = device.createSemaphore({});
}

void FrameContext::Destroy(const vk::Device device) const {
    if (inFlight) device.destroyFence(inFlight);
    if (commandBuffer) device.freeCommandBuffers(commandPool, commandBuffer);
    if (commandPool) device.destroyCommandPool(commandPool);
    if (renderFinished) device.destroySemaphore(renderFinished);
}
