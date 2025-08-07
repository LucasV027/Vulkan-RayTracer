#include "Renderer.h"

#include <imgui.h>

Renderer::Renderer(const std::shared_ptr<Window>& window,
                   const std::shared_ptr<VulkanContext>& context,
                   const std::shared_ptr<Raytracer>& raytracer) :
    window(window),
    vulkanContext(context),
    raytracer(raytracer) {
    swapchain = std::make_shared<Swapchain>(context, window);
    computePipeline = std::make_unique<ComputePipeline>(context, raytracer);
    graphicsPipeline = std::make_unique<GraphicsPipeline>(context, swapchain);
    graphicsPipeline->SetImageView(computePipeline->GetImageView());
    uiPipeline = std::make_unique<ImGuiPipeline>(context, window, swapchain);
}

Renderer::~Renderer() {
    if (vulkanContext->device) vulkanContext->device.waitIdle();
}

void Renderer::Draw() const {
    if (const auto fc = BeginFrame()) {
        computePipeline->Record(fc->commandBuffer);
        graphicsPipeline->Record(fc->commandBuffer);
        uiPipeline->Record(fc->commandBuffer);

        Submit(*fc);
        Present(*fc);
    } else {
        uiPipeline->End();
    }
}

void Renderer::Update() const {
    computePipeline->Update();
}

void Renderer::Begin() const {
    uiPipeline->Begin();
}

FrameContext* Renderer::BeginFrame() const {
    const auto result = AcquireNextImage();
    if (!result) {
        if (result.error() == AcquireError::Failed) vulkanContext->device.waitIdle();
        else Resize();
        swapchain->ResetCurrentImageIndex();
        return nullptr;
    }

    FrameContext* fc = *result;

    vulkanContext->device.resetCommandPool(fc->commandPool);

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

    vulkanContext->graphicsQueue.submit(submitInfo, fc.inFlight);
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
        const auto result = vulkanContext->graphicsQueue.presentKHR(presentInfo);
        if (result == vk::Result::eSuboptimalKHR) Resize();
    } catch (vk::OutOfDateKHRError&) {
        Resize();
    }
}

std::expected<FrameContext*, Renderer::AcquireError> Renderer::AcquireNextImage() const {
    auto acquireSemaphore = vulkanContext->device.createSemaphoreUnique({});

    uint32_t imageIndex;
    vk::Result result;

    try {
        std::tie(result, imageIndex) = vulkanContext->device.acquireNextImageKHR(
            swapchain->GetSwapchain(), UINT64_MAX, acquireSemaphore.get());
    } catch (vk::OutOfDateKHRError&) {
        return std::unexpected(AcquireError::OutOfDate);
    }

    if (result == vk::Result::eSuboptimalKHR) return std::unexpected(AcquireError::Suboptimal);
    if (result != vk::Result::eSuccess) return std::unexpected(AcquireError::Failed);

    swapchain->SetCurrentImageIndex(imageIndex);
    auto& frame = swapchain->GetCurrentFrameContext();

    if (frame.inFlight) {
        const auto waitResult = vulkanContext->device.waitForFences(frame.inFlight, true, UINT64_MAX);
        assert(waitResult == vk::Result::eSuccess);
        vulkanContext->device.resetFences(frame.inFlight);
    }

    frame.imageAvailable = std::move(acquireSemaphore);

    return &frame;
}

void Renderer::Resize() const {
    window->WaitWhileMinimized();

    vulkanContext->device.waitIdle();

    raytracer->Resize(window->GetSize().first, window->GetSize().second);
    swapchain->Recreate();
    computePipeline->Resize();
    graphicsPipeline->SetImageView(computePipeline->GetImageView());
}
