#include "Application.h"

#include <iostream>

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
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // No resize yet !

    window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        exit(1);
    }
}

void Application::InitVulkan() {
    CreateVkInstance();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    context.swapChainDimensions.width = width;
    context.swapChainDimensions.height = height;

    CreateSwapChain();
}

void Application::CreateVkInstance() {
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
        .pfnUserCallback = vkd::DebugCallback
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

    context.instance = vk::createInstance(instanceCreateInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(context.instance);

#ifndef NDEBUG
    context.debugCallback = context.instance.createDebugUtilsMessengerEXT(debugCreateInfo);
#endif
}

void Application::CreateSurface() {
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(context.instance, window, nullptr, &rawSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface.");
    }

    context.surface = vk::SurfaceKHR(rawSurface);
}

void Application::PickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> gpus = context.instance.enumeratePhysicalDevices();
    if (gpus.empty()) throw std::runtime_error("No Vulkan-compatible GPU found.");


    for (const auto& gpu : gpus) {
        vk::PhysicalDeviceProperties properties = gpu.getProperties();
        if (properties.apiVersion < vk::ApiVersion13) {
            LOGW("Physical device '{}' does not support Vulkan 1.3, skipping.", properties.deviceName.data());
            continue;
        }

        auto queueFamilies = gpu.getQueueFamilyProperties();

        for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
            const auto& queueFamily = queueFamilies[i];

            bool supportsGraphics = static_cast<bool>(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
            bool supportsPresent = false;

            if (supportsGraphics) {
                supportsPresent = gpu.getSurfaceSupportKHR(i, context.surface);
            }

            if (supportsGraphics && supportsPresent) {
                context.graphicsQueueIndex = i;
                context.gpu = gpu;

                LOGI("Selected GPU: '{}'", properties.deviceName.data());
                return;
            }
        }
    }

    throw std::runtime_error("No suitable GPU found (with Vulkan 1.3 + graphics + present support).");
}

void Application::CreateLogicalDevice() {
    // Check extensions support
    auto supportedExtensions = context.gpu.enumerateDeviceExtensionProperties();
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
    auto features = context.gpu.getFeatures2<
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

    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo queueCreateInfo{
        .queueFamilyIndex = static_cast<uint32_t>(context.graphicsQueueIndex),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &enabledFeatures.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

    context.device = context.gpu.createDevice(deviceCreateInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(context.device);
    context.queue = context.device.getQueue(context.graphicsQueueIndex, 0);
}

void Application::CreateSwapChain() {
    vk::SurfaceCapabilitiesKHR capabilities = context.gpu.getSurfaceCapabilitiesKHR(context.surface);
    auto formats = context.gpu.getSurfaceFormatsKHR(context.surface);
    auto presentModes = context.gpu.getSurfacePresentModesKHR(context.surface);

    vk::SurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    auto presentMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            presentMode = availablePresentMode;
            break;
        }
    }

    vk::Extent2D extent;
    extent.width = std::clamp(context.swapChainDimensions.width,
                              capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height = std::clamp(context.swapChainDimensions.height,
                               capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);

    const uint32_t imageCount = std::clamp(3u, capabilities.minImageCount,
                                           capabilities.maxImageCount > 0 ? capabilities.maxImageCount : 3u);

    vk::SwapchainCreateInfoKHR createInfo{
        .surface = context.surface,
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

    context.swapChain = context.device.createSwapchainKHR(createInfo);
    context.swapChainImages = context.device.getSwapchainImagesKHR(context.swapChain);
}

void Application::Cleanup() const {
    // Vulkan Cleanup
    // Don't release anything until the GPU is completely idle.
    if (context.device) {
        context.device.waitIdle();
    }

    if (context.swapChain) {
        context.device.destroySwapchainKHR(context.swapChain);
    }

    if (context.surface) {
        context.instance.destroySurfaceKHR(context.surface);
    }

    if (context.device) {
        context.device.destroy();
    }

    if (context.debugCallback) {
        context.instance.destroyDebugUtilsMessengerEXT(context.debugCallback);
    }

    // GLFW Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}
