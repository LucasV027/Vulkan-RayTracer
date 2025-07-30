#include "Renderer.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <set>

#include "Log.h"

Renderer::Renderer(const std::shared_ptr<VulkanContext>& context,
                   const std::shared_ptr<Window>& window) : context(context), window(window) {
    CreateSwapChain();
    CreateGraphicsPipeline();
    CreateVertexBuffer();

    InitImGUI();

    computePipeline = std::make_unique<ComputePipeline>(context);
}

Renderer::~Renderer() {
    // Don't release anything until the GPU is completely idle.
    if (context->device) context->device.waitIdle();

    computePipeline.reset();

    CleanupImGui();
    Cleanup();
}

void Renderer::Draw() {
    if (const auto fc = BeginFrame()) {
        computePipeline->UpdateUniform(14); // Test
        computePipeline->Dispatch(fc->commandBuffer, 16, 1, 1);

        Render(*fc);
        SubmitUI(*fc);
        Submit(*fc);
        Present(*fc);
    }
}

void Renderer::Begin() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
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

void Renderer::Render(const FrameContext& fc) const {
    constexpr vk::ClearValue clearValue{
        .color = std::array<float, 4>({{0.01f, 0.01f, 0.033f, 1.0f}}),
    };

    vk::RenderingAttachmentInfo colorAttachment{
        .imageView = swapChainImagesViews[fc.index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearValue
    };

    const vk::RenderingInfo renderingInfo{
        .renderArea = {
            .offset = {0, 0},
            .extent = {
                .width = swapChainDimensions.width,
                .height = swapChainDimensions.height
            }
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    fc.commandBuffer.beginRendering(renderingInfo);

    fc.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    const vk::Viewport vp{
        .width = static_cast<float>(swapChainDimensions.width),
        .height = static_cast<float>(swapChainDimensions.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    const vk::Rect2D scissor{
        .extent = {
            .width = swapChainDimensions.width,
            .height = swapChainDimensions.height
        }
    };

    fc.commandBuffer.setViewport(0, vp);
    fc.commandBuffer.setScissor(0, scissor);

    fc.commandBuffer.setCullMode(vk::CullModeFlagBits::eNone);
    fc.commandBuffer.setFrontFace(vk::FrontFace::eClockwise);
    fc.commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);

    fc.commandBuffer.bindVertexBuffers(0, vertexBuffer, {0});
    fc.commandBuffer.draw(QUAD.size(), 1, 0, 0);

    fc.commandBuffer.endRendering();
}

void Renderer::SubmitUI(const FrameContext& fc) const {
    ImGui::Render();

    constexpr vk::ClearValue clearValue{
        .color = std::array<float, 4>({{0.f, 0.f, 0.f, 0.f}}), // useless (only for API)
    };

    vk::RenderingAttachmentInfo colorAttachment{
        .imageView = swapChainImagesViews[fc.index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearValue
    };

    const vk::RenderingInfo renderingInfo{
        .renderArea = {
            .offset = {0, 0},
            .extent = {
                .width = swapChainDimensions.width,
                .height = swapChainDimensions.height
            }
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    fc.commandBuffer.beginRendering(renderingInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fc.commandBuffer);

    fc.commandBuffer.endRendering();
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

void Renderer::InitImGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    // Init GLFW
    ImGui_ImplGlfw_InitForVulkan(window->Handle(), true);

    // Init Vulkan backend
    vk::PipelineRenderingCreateInfo pipelineRenderingInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapChainDimensions.format,
    };

    ImGui_ImplVulkan_InitInfo initInfo{
        .Instance = context->instance,
        .PhysicalDevice = context->physicalDevice,
        .Device = context->device,
        .QueueFamily = context->graphicsQueueIndex,
        .Queue = context->graphicsQueue,
        .DescriptorPool = context->mainDescriptorPool,
        .MinImageCount = 2,
        .ImageCount = static_cast<uint32_t>(swapChainImages.size()),
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = pipelineRenderingInfo,
    };

    ImGui_ImplVulkan_Init(&initInfo);
}

void Renderer::Cleanup() {
    for (auto& perFrame : perFrames) perFrame.Destroy(context->device);
    perFrames.clear();

    if (graphicsPipeline) context->device.destroyPipeline(graphicsPipeline);
    if (graphicsPipelineLayout) context->device.destroyPipelineLayout(graphicsPipelineLayout);

    for (const vk::ImageView imageView : swapChainImagesViews) context->device.destroyImageView(imageView);

    if (swapChain) context->device.destroySwapchainKHR(swapChain);

    if (vertexBuffer) context->device.destroyBuffer(vertexBuffer);
    if (vertexBufferMemory) context->device.freeMemory(vertexBufferMemory);
}

void Renderer::CleanupImGui() const {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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

void Renderer::CreateGraphicsPipeline() {
    graphicsPipelineLayout = context->device.createPipelineLayout({});

    vk::VertexInputBindingDescription bindingDescription{
        .binding = 0,
        .stride = 2 * sizeof(float),
        .inputRate = vk::VertexInputRate::eVertex
    };

    // Define the vertex input attribute descriptions
    std::array<vk::VertexInputAttributeDescription, 1> attributeDescriptions = {
        {
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = 0
            },
        }
    };

    // Create the vertex input state
    vk::PipelineVertexInputStateCreateInfo vertexInput{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };


    // Specify we will use triangle lists to draw geometry.
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList
    };

    // Specify rasterization state.
    vk::PipelineRasterizationStateCreateInfo raster{
        .polygonMode = vk::PolygonMode::eFill,
        .lineWidth = 1.0f
    };

    // Specify that these states will be dynamic, i.e. not part of pipeline state object.
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
        vk::DynamicState::eCullMode,
        vk::DynamicState::eFrontFace,
        vk::DynamicState::ePrimitiveTopology
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };


    // Our attachment will write to all color channels, but no blending is enabled.
    vk::PipelineColorBlendAttachmentState blendAttachment{
        .colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo blend{
        .attachmentCount = 1,
        .pAttachments = &blendAttachment
    };

    // We will have one viewport and scissor box.
    vk::PipelineViewportStateCreateInfo viewport{
        .viewportCount = 1,
        .scissorCount = 1
    };

    // Disable all depth testing.
    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthCompareOp = vk::CompareOp::eAlways
    };

    // No multisampling.
    vk::PipelineMultisampleStateCreateInfo multisample{
        .rasterizationSamples = vk::SampleCountFlagBits::e1
    };


    auto vertShaderModule = vkHelpers::CreateShaderModule(context->device, "../shaders/main.vert.spv");
    auto fragShaderModule = vkHelpers::CreateShaderModule(context->device, "../shaders/main.frag.spv");

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {
        {
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = vertShaderModule.get(),
                .pName = "main"
            },
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = fragShaderModule.get(),
                .pName = "main"
            }
        }
    };

    // Pipeline rendering info (for dynamic rendering).
    vk::PipelineRenderingCreateInfo pipelineRenderingInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapChainDimensions.format
    };

    // Create the graphics pipeline.
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
        .pNext = &pipelineRenderingInfo,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewport,
        .pRasterizationState = &raster,
        .pMultisampleState = &multisample,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &blend,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = graphicsPipelineLayout,
        // We need to specify the pipeline layout description up front as well.
        .renderPass = nullptr, // Since we are using dynamic rendering this will set as null
        .subpass = 0,
    };

    vk::Result result;
    std::tie(result, graphicsPipeline) = context->device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
    if (result != vk::Result::eSuccess) throw std::runtime_error("failed to create graphics pipeline");
}

void Renderer::CreateVertexBuffer() {
    const vk::DeviceSize bufferSize = sizeof(QUAD[0]) * QUAD.size();

    const vk::BufferCreateInfo vertexBufferCreateInfo{
        .size = bufferSize,
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };

    vertexBuffer = context->device.createBuffer(vertexBufferCreateInfo);

    const vk::MemoryRequirements memoryRequirements = context->device.getBufferMemoryRequirements(vertexBuffer);

    const vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex =
        vkHelpers::FindMemoryType(context->physicalDevice, memoryRequirements.memoryTypeBits,
                                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    };

    vertexBufferMemory = context->device.allocateMemory(allocInfo);

    context->device.bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);

    void* data = context->device.mapMemory(vertexBufferMemory, 0, bufferSize);
    memcpy(data, QUAD.data(), bufferSize);
    context->device.unmapMemory(vertexBufferMemory);
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
