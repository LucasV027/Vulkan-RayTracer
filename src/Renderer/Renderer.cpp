#include "Renderer.h"

#include <imgui.h>
#include <set>

#include "Log.h"

Renderer::Renderer(const std::shared_ptr<VulkanContext>& context,
                   const std::shared_ptr<Window>& window) :
    context(context),
    window(window) {
    swapchain = std::make_shared<Swapchain>(context, window);

    auto [windowWidth,windowHeight] = window->GetSize();
    RaytracingContext::Config rtConfig{
        .width = windowWidth,
        .height = windowHeight,
        .outputFormat = vk::Format::eR32G32B32A32Sfloat
    };
    rtContext = std::make_shared<RaytracingContext>(context, rtConfig);

    computePipeline = std::make_unique<ComputePipeline>(context, rtContext);
    graphicsPipeline = std::make_unique<GraphicsPipeline>(context, swapchain, rtContext);
    uiPipeline = std::make_unique<ImGuiPipeline>(context, window, swapchain);
}

Renderer::~Renderer() {
    if (context->device) context->device.waitIdle();
}

void Renderer::Draw() const {
    if (const auto fc = BeginFrame()) {
        rtContext->TransitionForCompute(fc->commandBuffer);
        computePipeline->Record(fc->commandBuffer);
        rtContext->TransitionForDisplay(fc->commandBuffer);
        graphicsPipeline->Record(fc->commandBuffer);
        uiPipeline->Record(fc->commandBuffer);

        Submit(*fc);
        Present(*fc);
    } else {
        uiPipeline->End();
    }
}

void Renderer::Begin() const {
    uiPipeline->Begin();
}

FrameContext* Renderer::BeginFrame() const {
    const auto result = AcquireNextImage();
    if (!result) {
        if (result.error() == AcquireError::Failed) context->device.waitIdle();
        else Resize();
        swapchain->ResetCurrentImageIndex();
        return nullptr;
    }

    FrameContext* fc = *result;

    context->device.resetCommandPool(fc->commandPool);

    fc->commandBuffer.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    });

    vkHelpers::TransitionImageLayout(
        fc->commandBuffer,
        swapchain->GetImages()[fc->index],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    return fc;
}

void Renderer::Submit(const FrameContext& fc) const {
    vkHelpers::TransitionImageLayout(fc.commandBuffer,
                                     swapchain->GetImages()[fc.index],
                                     vk::ImageLayout::eColorAttachmentOptimal,
                                     vk::ImageLayout::ePresentSrcKHR,
                                     vk::AccessFlagBits2::eColorAttachmentWrite,
                                     {},
                                     vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                     vk::PipelineStageFlagBits2::eBottomOfPipe
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

void Renderer::Present(const FrameContext& fc) const {
    auto sc = swapchain->GetSwapchain();
    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &fc.renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &sc,
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

std::expected<FrameContext*, Renderer::AcquireError> Renderer::AcquireNextImage() const {
    auto acquireSemaphore = context->device.createSemaphoreUnique({});

    uint32_t imageIndex;
    vk::Result result;

    try {
        std::tie(result, imageIndex) = context->device.acquireNextImageKHR(
            swapchain->GetSwapchain(), UINT64_MAX, acquireSemaphore.get());
    } catch (vk::OutOfDateKHRError&) {
        return std::unexpected(AcquireError::OutOfDate);
    }

    if (result == vk::Result::eSuboptimalKHR) return std::unexpected(AcquireError::Suboptimal);
    if (result != vk::Result::eSuccess) return std::unexpected(AcquireError::Failed);

    swapchain->SetCurrentImageIndex(imageIndex);
    auto& frame = swapchain->GetCurrentFrameContext();

    if (frame.inFlight) {
        const auto waitResult = context->device.waitForFences(frame.inFlight, true, UINT64_MAX);
        assert(waitResult == vk::Result::eSuccess);
        context->device.resetFences(frame.inFlight);
    }

    frame.imageAvailable = std::move(acquireSemaphore);

    return &frame;
}

void Renderer::Resize() const {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        std::tie(width, height) = window->GetFrameBufferSize();
        glfwWaitEvents();
    }

    context->device.waitIdle();

    const auto surfaceProperties = context->physicalDevice.getSurfaceCapabilitiesKHR(context->surface);

    auto [currentWidth, currentHeight] = swapchain->GetExtent();
    const bool dimensionsChanged =
        surfaceProperties.currentExtent.width != currentWidth ||
        surfaceProperties.currentExtent.height != currentHeight;

    if (dimensionsChanged) swapchain->Recreate();
}
