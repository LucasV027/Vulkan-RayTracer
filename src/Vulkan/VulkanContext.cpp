#include "VulkanContext.h"

#include <set>

#include "Core/Log.h"

VulkanContext::VulkanContext(const std::shared_ptr<Window>& window) : window(window) {
    CreateInstance();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateDescriptorPool();
    CreateCommandPool();
}

VulkanContext::~VulkanContext() {
    if (device) {
        if (mainDescriptorPool) device.destroyDescriptorPool(mainDescriptorPool);
        if (commandPool) device.destroyCommandPool(commandPool);

        device.destroy();
    }

    if (instance) {
        if (surface) instance.destroySurfaceKHR(surface);
        if (debugCallback) instance.destroyDebugUtilsMessengerEXT(debugCallback, nullptr);

        instance.destroy();
    }
}

vk::CommandBuffer VulkanContext::BeginSingleTimeCommands() const {
    const vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = commandPool,
        .commandBufferCount = 1,
    };

    vk::CommandBuffer commandBuffer;
    if (device.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    constexpr vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void VulkanContext::EndSingleTimeCommands(vk::CommandBuffer commandBuffer) const {
    commandBuffer.end();

    const vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    const auto result = graphicsQueue.submit(1, &submitInfo, nullptr);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit command buffer !");
    }

    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void VulkanContext::CreateInstance() {
    static vk::detail::DynamicLoader loader;
    const auto vkGetInstanceProcAddr = loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo appInfo{
        .pApplicationName = window->GetTitle(),
        .pEngineName = "",
        .apiVersion = VK_MAKE_VERSION(1, 3, 0)
    };

    std::vector extensions = window->GetRequiredSurfaceExtensions();
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

    instance = vk::createInstance(instanceCreateInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

#ifndef NDEBUG
    debugCallback = instance.createDebugUtilsMessengerEXT(debugCreateInfo);
#endif
}

void VulkanContext::CreateSurface() {
    surface = window->CreateSurface(instance);
}

void VulkanContext::PickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> gpus = instance.enumeratePhysicalDevices();
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
            const bool supportsGraphics = static_cast<bool>(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);

            if (supportsGraphics && gpu.getSurfaceSupportKHR(i, surface)) {
                graphicsQueueIndex = i;
                physicalDevice = gpu;
                LOGI("Selected GPU: '{}'", properties.deviceName.data());
                return;
            }
        }
    }

    throw std::runtime_error("No suitable GPU found (with Vulkan 1.3 + graphics support).");
}

void VulkanContext::CreateLogicalDevice() {
    // Check extensions support
    auto supportedExtensions = physicalDevice.enumerateDeviceExtensionProperties();
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
    auto features = physicalDevice.getFeatures2<
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
        .queueFamilyIndex = graphicsQueueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &enabledFeatures.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1u,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };


    device = physicalDevice.createDevice(deviceCreateInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
    graphicsQueue = device.getQueue(graphicsQueueIndex, 0);
}

void VulkanContext::CreateDescriptorPool() {
    std::array poolSizes = {
        vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic, 1000},
        vk::DescriptorPoolSize{vk::DescriptorType::eInputAttachment, 1000}
    };

    const vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1000,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    mainDescriptorPool = device.createDescriptorPool(poolInfo);
}

void VulkanContext::CreateCommandPool() {
    const vk::CommandPoolCreateInfo poolInfo = {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = graphicsQueueIndex,
    };

    commandPool = device.createCommandPool(poolInfo);
}
