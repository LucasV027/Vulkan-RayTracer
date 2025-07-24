#include "Application.h"

#include <set>

#include "Utils.h"
#include "Log.h"

Application::Application() {
    try {
        window.Create(width, height, appName);
        InitVulkan();
    } catch (const std::exception& e) {
        LOGE("Failed to initialize application: {}", e.what());
        Cleanup();
        std::exit(EXIT_FAILURE);
    }
}

void Application::Run() {
    while (running) {
        glfwPollEvents();
        Update();
        running = !window.ShouldClose();
    }
}

Application::~Application() {
    Cleanup();
}

void Application::InitVulkan() {
    CreateInstance();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateGraphicsPipeline();
    CreateVertexBuffer();
}

void Application::CreateInstance() {
    static vk::detail::DynamicLoader loader;
    const auto vkGetInstanceProcAddr = loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo appInfo{
        .pApplicationName = appName.c_str(),
        .pEngineName = "",
        .apiVersion = VK_MAKE_VERSION(1, 3, 0)
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    std::vector<const char*> requestedInstanceLayers;

#ifndef NDEBUG
    constexpr auto validationLayer = "VK_LAYER_KHRONOS_validation";
    const auto supportedLayers = vk::enumerateInstanceLayerProperties();
    for (const auto& layer : supportedLayers) {
        if (strcmp(layer.layerName, validationLayer) == 0) {
            requestedInstanceLayers.push_back(validationLayer);
            break;
        }
    }

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    constexpr vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{
        .messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        .pfnUserCallback = vkHelpers::DebugCallback
    };
#endif

    vk::InstanceCreateInfo instanceCreateInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requestedInstanceLayers.size()),
        .ppEnabledLayerNames = requestedInstanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

#ifndef NDEBUG
    instanceCreateInfo.pNext = &debugCreateInfo;
#endif

    ctx.instance = vk::createInstance(instanceCreateInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(ctx.instance);

#ifndef NDEBUG
    ctx.debugCallback = ctx.instance.createDebugUtilsMessengerEXT(debugCreateInfo);
#endif
}

void Application::CreateSurface() {
    // VkSurfaceKHR rawSurface;
    // if (glfwCreateWindowSurface(ctx.instance, window, nullptr, &rawSurface) != VK_SUCCESS) {
    //     throw std::runtime_error("Failed to create window surface.");
    // }

    // ctx.surface = vk::SurfaceKHR(rawSurface);

    ctx.surface = window.CreateSurface(ctx.instance);
}

void Application::PickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> gpus = ctx.instance.enumeratePhysicalDevices();
    if (gpus.empty()) throw std::runtime_error("No Vulkan-compatible GPU found.");


    for (const auto& gpu : gpus) {
        vk::PhysicalDeviceProperties properties = gpu.getProperties();
        if (properties.apiVersion < vk::ApiVersion13) {
            LOGW("Physical device '{}' does not support Vulkan 1.3, skipping.", properties.deviceName.data());
            continue;
        }

        auto queueFamilies = gpu.getQueueFamilyProperties();

        bool foundGraphics = false;
        bool foundCompute = false;

        for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
            const auto& queueFamily = queueFamilies[i];
            const bool supportsGraphics = static_cast<bool>(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
            const bool supportsCompute = static_cast<bool>(queueFamily.queueFlags & vk::QueueFlagBits::eCompute);

            if (supportsGraphics && gpu.getSurfaceSupportKHR(i, ctx.surface) && !foundGraphics) {
                ctx.graphicsQueueIndex = i;
                foundGraphics = true;
            }

            // Prefer a compute-only queue
            if (supportsCompute && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && !foundCompute) {
                ctx.computeQueueIndex = i;
                foundCompute = true;
            }
        }

        if (!foundCompute && foundGraphics) {
            ctx.computeQueueIndex = ctx.graphicsQueueIndex;
            foundCompute = true;
            LOGI("No dedicated compute queue found, fallback to graphics queue");
        }

        if (foundGraphics && foundCompute) {
            ctx.gpu = gpu;
            LOGI("Selected GPU: '{}'", properties.deviceName.data());
            return;
        }
    }

    throw std::runtime_error("No suitable GPU found (with Vulkan 1.3 + graphics + compute support).");
}

void Application::CreateLogicalDevice() {
    // Check extensions support
    auto supportedExtensions = ctx.gpu.enumerateDeviceExtensionProperties();
    std::vector<const char*> requiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    for (const char* ext : requiredExtensions) {
        bool found = std::ranges::any_of(supportedExtensions, [&](const auto& e) {
            return strcmp(e.extensionName, ext) == 0;
        });
        if (!found) {
            throw std::runtime_error(std::string("Missing required extension: ") + ext);
        }
    }

    // Check supported features
    auto features = ctx.gpu.getFeatures2<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    if (!features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering ||
        !features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 ||
        !features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState) {
        throw std::runtime_error("Required Vulkan features not supported.");
    }

    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> enabledFeatures = {
        {}, {.synchronization2 = true, .dynamicRendering = true}, {.extendedDynamicState = true}
    };

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        ctx.graphicsQueueIndex, ctx.computeQueueIndex
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo{
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });
    }

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &enabledFeatures.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };


    ctx.device = ctx.gpu.createDevice(deviceCreateInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(ctx.device);
    ctx.graphicsQueue = ctx.device.getQueue(ctx.graphicsQueueIndex, 0);
    ctx.computeQueue = ctx.device.getQueue(ctx.computeQueueIndex, 0);
}

void Application::CreateSwapChain() {
    vk::SurfaceCapabilitiesKHR capabilities = ctx.gpu.getSurfaceCapabilitiesKHR(ctx.surface);
    auto formats = ctx.gpu.getSurfaceFormatsKHR(ctx.surface);
    auto presentModes = ctx.gpu.getSurfacePresentModesKHR(ctx.surface);

    vk::SurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    ctx.swapChainDimensions.format = surfaceFormat.format;

    auto presentMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            presentMode = availablePresentMode;
            break;
        }
    }

    auto [width, height] = window.GetSize();
    ctx.swapChainDimensions.width = width;
    ctx.swapChainDimensions.height = height;

    vk::Extent2D extent;
    if (capabilities.currentExtent.width == UINT32_MAX) {
        extent.width = std::clamp(ctx.swapChainDimensions.width,
                                  capabilities.minImageExtent.width,
                                  capabilities.maxImageExtent.width);
        extent.height = std::clamp(ctx.swapChainDimensions.height,
                                   capabilities.minImageExtent.height,
                                   capabilities.maxImageExtent.height);
    } else {
        extent = capabilities.currentExtent;
    }


    uint32_t imageCount = 3;
    if (capabilities.maxImageCount > 0)
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    imageCount = std::max(imageCount, capabilities.minImageCount);

    auto oldSwapChain = ctx.swapChain;

    vk::SwapchainCreateInfoKHR createInfo{
        .surface = ctx.surface,
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

    ctx.swapChain = ctx.device.createSwapchainKHR(createInfo);
    ctx.swapChainImages = ctx.device.getSwapchainImagesKHR(ctx.swapChain);

    if (oldSwapChain) {
        for (vk::ImageView imageView : ctx.swapChainImagesViews) ctx.device.destroyImageView(imageView);
        ctx.swapChainImagesViews.clear();

        for (auto& perFrame : ctx.perFrames) perFrame.Destroy(ctx.device);

        ctx.device.destroySwapchainKHR(oldSwapChain);
    }

    for (auto const& swapChainImage : ctx.swapChainImages) {
        vk::ImageViewCreateInfo viewCreateInfo{
            .image = swapChainImage,
            .viewType = vk::ImageViewType::e2D,
            .format = ctx.swapChainDimensions.format,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        ctx.swapChainImagesViews.push_back(ctx.device.createImageView(viewCreateInfo));
    }

    ctx.perFrames.clear();
    ctx.perFrames.resize(ctx.swapChainImages.size());

    for (auto& perFrame : ctx.perFrames) {
        perFrame.Init(ctx.device, ctx.graphicsQueueIndex);
    }
}

void Application::CreateGraphicsPipeline() {
    ctx.graphicsPipelineLayout = ctx.device.createPipelineLayout({});

    vk::VertexInputBindingDescription bindingDescription{
        .binding = 0,
        .stride = 5 * sizeof(float),
        .inputRate = vk::VertexInputRate::eVertex
    };

    // Define the vertex input attribute descriptions
    std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = {
        {
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = 0
            },
            // position
            {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = 2 * sizeof(float),
            } // color
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


    vk::ShaderModule vertShaderModule = vkHelpers::CreateShaderModule(ctx.device, "../shaders/main.vert.spv");
    vk::ShaderModule fragShaderModule = vkHelpers::CreateShaderModule(ctx.device, "../shaders/main.frag.spv");

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {
        {
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = vertShaderModule,
                .pName = "main"
            },
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = fragShaderModule,
                .pName = "main"
            }
        }
    };

    // Pipeline rendering info (for dynamic rendering).
    vk::PipelineRenderingCreateInfo pipelineRenderingInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &ctx.swapChainDimensions.format
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
        .layout = ctx.graphicsPipelineLayout,
        // We need to specify the pipeline layout description up front as well.
        .renderPass = nullptr, // Since we are using dynamic rendering this will set as null
        .subpass = 0,
    };

    vk::Result result;
    std::tie(result, ctx.graphicsPipeline) = ctx.device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
    if (result != vk::Result::eSuccess) throw std::runtime_error("failed to create graphics pipeline");

    for (auto& shaderStage : shaderStages) {
        ctx.device.destroyShaderModule(shaderStage.module);
    }
}

void Application::CreateVertexBuffer() {
    const vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    const vk::BufferCreateInfo vertexBufferCreateInfo{
        .size = bufferSize,
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };

    ctx.vertexBuffer = ctx.device.createBuffer(vertexBufferCreateInfo);

    const vk::MemoryRequirements memoryRequirements = ctx.device.getBufferMemoryRequirements(ctx.vertexBuffer);

    auto FindMemoryType = [](const vk::PhysicalDevice physicalDevice,
                             const uint32_t typeFilter,
                             const vk::MemoryPropertyFlags properties) {
        // Structure to hold the physical device's memory properties
        const auto memProperties = physicalDevice.getMemoryProperties();

        // Iterate over all memory types available on the physical device
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            // Check if the current memory type is acceptable based on the type_filter
            // The type_filter is a bitmask where each bit represents a memory type that is suitable
            if (typeFilter & (1 << i)) {
                // Check if the memory type has all the desired property flags
                // properties is a bitmask of the required memory properties
                if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    // Found a suitable memory type; return its index
                    return i;
                }
            }
        }

        // If no suitable memory type was found, throw an exception
        throw std::runtime_error("Failed to find suitable memory type.");
    };

    const vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex =
        FindMemoryType(ctx.gpu, memoryRequirements.memoryTypeBits,
                       vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    };

    ctx.vertexBufferMemory = ctx.device.allocateMemory(allocInfo);

    ctx.device.bindBufferMemory(ctx.vertexBuffer, ctx.vertexBufferMemory, 0);

    void* data = ctx.device.mapMemory(ctx.vertexBufferMemory, 0, bufferSize);
    memcpy(data, vertices.data(), bufferSize);
    ctx.device.unmapMemory(ctx.vertexBufferMemory);
}

std::expected<uint32_t, Application::AcquireError> Application::AcquireNextImage() {
    auto acquireSemaphore = ctx.device.createSemaphoreUnique({});

    uint32_t imageIndex;
    vk::Result result;
    try {
        std::tie(result, imageIndex) = ctx.device.acquireNextImageKHR(
            ctx.swapChain, UINT64_MAX, acquireSemaphore.get());
    } catch (vk::OutOfDateKHRError&) {
        return std::unexpected(AcquireError::OutOfDate);
    }

    if (result == vk::Result::eSuboptimalKHR) return std::unexpected(AcquireError::Suboptimal);
    if (result != vk::Result::eSuccess) return std::unexpected(AcquireError::Failed);

    if (ctx.perFrames[imageIndex].inFlight) {
        const auto waitResult = ctx.device.waitForFences(
            ctx.perFrames[imageIndex].inFlight, true, UINT64_MAX);
        assert(waitResult == vk::Result::eSuccess);
        ctx.device.resetFences(ctx.perFrames[imageIndex].inFlight);
    }

    ctx.perFrames[imageIndex].imageAvailable = std::move(acquireSemaphore);

    return imageIndex;
}

void Application::Render(const uint32_t swapChainIndex) {
    ctx.device.resetCommandPool(ctx.perFrames[swapChainIndex].commandPool);

    vk::CommandBuffer cmd = ctx.perFrames[swapChainIndex].commandBuffer;
    constexpr vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

    cmd.begin(beginInfo);
    vkHelpers::TransitionImageLayout(cmd,
                                     ctx.swapChainImages[swapChainIndex],
                                     vk::ImageLayout::eUndefined,
                                     vk::ImageLayout::eColorAttachmentOptimal,
                                     {}, // srcAccessMask (no need to wait for previous operations)
                                     vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
                                     vk::PipelineStageFlagBits2::eTopOfPipe, // srcStage
                                     vk::PipelineStageFlagBits2::eColorAttachmentOutput // dstStage
    );

    constexpr vk::ClearValue clearValue{
        .color = std::array<float, 4>({{0.01f, 0.01f, 0.033f, 1.0f}}),
    };

    // Set up the rendering attachment info
    vk::RenderingAttachmentInfo colorAttachment{
        .imageView = ctx.swapChainImagesViews[swapChainIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearValue
    };

    // Begin rendering
    const vk::RenderingInfo renderingInfo{
        .renderArea = {
            // Initialize the nested `VkRect2D` structure
            .offset = {0, 0}, // Initialize the `VkOffset2D` inside `renderArea`
            .extent = {
                // Initialize the `VkExtent2D` inside `renderArea`
                .width = ctx.swapChainDimensions.width,
                .height = ctx.swapChainDimensions.height
            }
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    cmd.beginRendering(renderingInfo);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.graphicsPipeline);

    // Set viewport dynamically
    const vk::Viewport vp{
        .width = static_cast<float>(ctx.swapChainDimensions.width),
        .height = static_cast<float>(ctx.swapChainDimensions.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    cmd.setViewport(0, vp);

    // Set scissor dynamically
    const vk::Rect2D scissor{
        .extent = {
            .width = ctx.swapChainDimensions.width,
            .height = ctx.swapChainDimensions.height
        }
    };

    cmd.setScissor(0, scissor);
    cmd.setCullMode(vk::CullModeFlagBits::eNone);
    cmd.setFrontFace(vk::FrontFace::eClockwise);
    cmd.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);

    cmd.bindVertexBuffers(0, ctx.vertexBuffer, {0});
    cmd.draw(vertices.size(), 1, 0, 0);

    cmd.endRendering();

    vkHelpers::TransitionImageLayout(cmd,
                                     ctx.swapChainImages[swapChainIndex],
                                     vk::ImageLayout::eColorAttachmentOptimal,
                                     vk::ImageLayout::ePresentSrcKHR,
                                     vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
                                     {},                                                 // dstAccessMask
                                     vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
                                     vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
    );

    cmd.end();

    vk::PipelineStageFlags waitStage = {vk::PipelineStageFlagBits::eTopOfPipe};

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &ctx.perFrames[swapChainIndex].imageAvailable.get(),
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &ctx.perFrames[swapChainIndex].renderFinished
    };

    ctx.graphicsQueue.submit(submitInfo, ctx.perFrames[swapChainIndex].inFlight);
}

bool Application::PresentImage(uint32_t swapChainIndex) {
    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &ctx.perFrames[swapChainIndex].renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &ctx.swapChain,
        .pImageIndices = &swapChainIndex
    };

    try {
        const auto result = ctx.graphicsQueue.presentKHR(presentInfo);
        if (result == vk::Result::eSuboptimalKHR) {
            return false;
        }
    } catch (vk::OutOfDateKHRError&) {
        return false;
    }

    return true;
}

void Application::Resize() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        std::tie(width, height) = window.GetFrameBufferSize();
        glfwWaitEvents();
    }

    ctx.device.waitIdle();

    const auto surfaceProperties = ctx.gpu.getSurfaceCapabilitiesKHR(ctx.surface);

    const bool dimensionsChanged =
        surfaceProperties.currentExtent.width != ctx.swapChainDimensions.width ||
        surfaceProperties.currentExtent.height != ctx.swapChainDimensions.height;

    if (dimensionsChanged) {
        CreateSwapChain();
    }
}

void Application::Update() {
    const auto acquireResult = AcquireNextImage();

    if (!acquireResult) {
        if (acquireResult.error() == AcquireError::Failed) {
            ctx.device.waitIdle();
            LOGE("Acquire failed");
            return;
        }

        Resize();
        return;
    }

    Render(acquireResult.value());

    const auto presentResult = PresentImage(acquireResult.value());
    if (!presentResult) {
        Resize();
        return;
    }
}

void Application::Cleanup() {
    VulkanCleanup();
}

void Application::VulkanCleanup() {
    // Don't release anything until the GPU is completely idle.
    if (ctx.device) {
        ctx.device.waitIdle();
    }

    for (auto& perFrame : ctx.perFrames) {
        perFrame.Destroy(ctx.device);
    }

    ctx.perFrames.clear();

    if (ctx.graphicsPipeline) {
        ctx.device.destroyPipeline(ctx.graphicsPipeline);
    }

    if (ctx.graphicsPipelineLayout) {
        ctx.device.destroyPipelineLayout(ctx.graphicsPipelineLayout);
    }

    for (const vk::ImageView imageView : ctx.swapChainImagesViews) {
        ctx.device.destroyImageView(imageView);
    }

    if (ctx.swapChain) {
        ctx.device.destroySwapchainKHR(ctx.swapChain);
    }

    if (ctx.surface) {
        ctx.instance.destroySurfaceKHR(ctx.surface);
    }

    if (ctx.vertexBuffer) {
        ctx.device.destroyBuffer(ctx.vertexBuffer);
    }

    if (ctx.vertexBufferMemory) {
        ctx.device.freeMemory(ctx.vertexBufferMemory);
    }

    if (ctx.device) {
        ctx.device.destroy();
    }

    if (ctx.debugCallback) {
        ctx.instance.destroyDebugUtilsMessengerEXT(ctx.debugCallback);
    }
}

void Application::PerFrame::Init(const vk::Device device, const uint32_t graphicsQueueIndex) {
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

void Application::PerFrame::Destroy(const vk::Device device) const {
    if (inFlight) device.destroyFence(inFlight);
    if (commandBuffer) device.freeCommandBuffers(commandPool, commandBuffer);
    if (commandPool) device.destroyCommandPool(commandPool);
    if (renderFinished) device.destroySemaphore(renderFinished);
}
