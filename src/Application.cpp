#include "Application.h"

#include <set>

#include "Utils.h"
#include "Log.h"

Application::Application() {
    InitWindow();
    InitVulkan();
}

void Application::Run() const {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

Application::~Application() {
    Cleanup();
}

void Application::InitWindow() {
    if (!glfwInit()) {
        LOGE("Failed to initialize GLFW");
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // No resize yet !

    window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        LOGE("Failed to create GLFW window");
        exit(1);
    }
}

void Application::InitVulkan() {
    CreateInstance();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ctx.graphics.swapChain.dimensions.width = width;
    ctx.graphics.swapChain.dimensions.height = height;

    CreateSwapChain();

    CreateGraphicsPipeline();
}

void Application::CreateInstance() {
    static vk::detail::DynamicLoader loader;
    auto vkGetInstanceProcAddr = loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo appInfo{
        .pApplicationName = name.c_str(),
        .pEngineName = "",
        .apiVersion = VK_MAKE_VERSION(1, 3, 0)
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    std::vector<const char*> requestedInstanceLayers;

#ifndef NDEBUG
    constexpr const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    const auto supportedLayers = vk::enumerateInstanceLayerProperties();
    for (const auto& layer : supportedLayers) {
        if (strcmp(layer.layerName, validationLayer) == 0) {
            requestedInstanceLayers.push_back(validationLayer);
            break;
        }
    }

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{
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
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(ctx.instance, window, nullptr, &rawSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface.");
    }

    ctx.surface = vk::SurfaceKHR(rawSurface);
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
                ctx.graphics.queueIndex = i;
                foundGraphics = true;
            }

            // Prefer a compute-only queue
            if (supportsCompute && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && !foundCompute) {
                ctx.compute.queueIndex = i;
                foundCompute = true;
            }
        }

        if (!foundCompute && foundGraphics) {
            ctx.compute.queueIndex = ctx.graphics.queueIndex;
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
        ctx.graphics.queueIndex, ctx.compute.queueIndex
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
    ctx.graphics.queue = ctx.device.getQueue(ctx.graphics.queueIndex, 0);
    ctx.compute.queue = ctx.device.getQueue(ctx.compute.queueIndex, 0);
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

    ctx.graphics.swapChain.dimensions.format = surfaceFormat.format;

    auto presentMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            presentMode = availablePresentMode;
            break;
        }
    }

    vk::Extent2D extent;
    if (capabilities.currentExtent.width == UINT32_MAX) {
        extent.width = std::clamp(ctx.graphics.swapChain.dimensions.width,
                                  capabilities.minImageExtent.width,
                                  capabilities.maxImageExtent.width);
        extent.height = std::clamp(ctx.graphics.swapChain.dimensions.height,
                                   capabilities.minImageExtent.height,
                                   capabilities.maxImageExtent.height);
    } else {
        extent = capabilities.currentExtent;
    }


    uint32_t imageCount = 3;
    if (capabilities.maxImageCount > 0)
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    imageCount = std::max(imageCount, capabilities.minImageCount);

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
        .clipped = true
    };

    ctx.graphics.swapChain.handle = ctx.device.createSwapchainKHR(createInfo);
    ctx.graphics.swapChain.images = ctx.device.getSwapchainImagesKHR(ctx.graphics.swapChain.handle);

    for (auto const& swapChainImage : ctx.graphics.swapChain.images) {
        vk::ImageViewCreateInfo viewCreateInfo{
            .image = swapChainImage,
            .viewType = vk::ImageViewType::e2D,
            .format = ctx.graphics.swapChain.dimensions.format,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        ctx.graphics.swapChain.imagesViews.push_back(ctx.device.createImageView(viewCreateInfo));
    }

    ctx.graphics.swapChain.perFrames.clear();
    ctx.graphics.swapChain.perFrames.resize(ctx.graphics.swapChain.images.size());

    for (auto& [commandPool, commandBuffer] : ctx.graphics.swapChain.perFrames) {
        vk::CommandPoolCreateInfo commandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = ctx.graphics.queueIndex,
        };

        commandPool = ctx.device.createCommandPool(commandPoolCreateInfo);

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
            .commandPool = commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        commandBuffer = ctx.device.allocateCommandBuffers(commandBufferAllocateInfo)[0];
    }
}

void Application::CreateGraphicsPipeline() {
    ctx.graphics.pipelineLayout = ctx.device.createPipelineLayout({});

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
        .pColorAttachmentFormats = &ctx.graphics.swapChain.dimensions.format
    };

    // Create the graphics pipeline.
    vk::GraphicsPipelineCreateInfo pipeline_create_info{
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
        .layout = ctx.graphics.pipelineLayout,
        // We need to specify the pipeline layout description up front as well.
        .renderPass = VK_NULL_HANDLE, // Since we are using dynamic rendering this will set as null
        .subpass = 0,
    };

    vk::Result result;
    std::tie(result, ctx.graphics.pipeline) = ctx.device.createGraphicsPipeline(nullptr, pipeline_create_info);
    assert(result == vk::Result::eSuccess);

    for (auto& shaderStage : shaderStages) {
        ctx.device.destroyShaderModule(shaderStage.module);
    }
}

void Application::Cleanup() const {
    // Vulkan Cleanup
    // Don't release anything until the GPU is completely idle.
    if (ctx.device) {
        ctx.device.waitIdle();
    }

    for (auto& [commandPool, commandBuffer] : ctx.graphics.swapChain.perFrames) {
        if (commandBuffer) {
            ctx.device.freeCommandBuffers(commandPool, commandBuffer);
        }

        if (commandPool) {
            ctx.device.destroyCommandPool(commandPool);
        }
    }


    if (ctx.graphics.pipeline) {
        ctx.device.destroyPipeline(ctx.graphics.pipeline);
    }

    if (ctx.graphics.pipelineLayout) {
        ctx.device.destroyPipelineLayout(ctx.graphics.pipelineLayout);
    }

    for (vk::ImageView imageView : ctx.graphics.swapChain.imagesViews) {
        ctx.device.destroyImageView(imageView);
    }

    if (ctx.graphics.swapChain.handle) {
        ctx.device.destroySwapchainKHR(ctx.graphics.swapChain.handle);
    }

    if (ctx.surface) {
        ctx.instance.destroySurfaceKHR(ctx.surface);
    }

    if (ctx.device) {
        ctx.device.destroy();
    }

    if (ctx.debugCallback) {
        ctx.instance.destroyDebugUtilsMessengerEXT(ctx.debugCallback);
    }

    // GLFW Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}
