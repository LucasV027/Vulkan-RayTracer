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

void Application::Cleanup() const {
    // Vulkan Cleanup
    if (context.debugCallback) {
        context.instance.destroyDebugUtilsMessengerEXT(context.debugCallback);
    }

    // GLFW Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}
