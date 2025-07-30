#include "ImGuiPipeline.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

ImGuiPipeline::ImGuiPipeline(const std::shared_ptr<VulkanContext>& context,
                             const std::shared_ptr<Window>& window,
                             const vk::Format format,
                             const uint32_t imageCount)
    : context(context), window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    // Init GLFW
    ImGui_ImplGlfw_InitForVulkan(window->Handle(), true);

    // Init Vulkan backend
    vk::PipelineRenderingCreateInfo pipelineRenderingInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &format,
    };

    ImGui_ImplVulkan_InitInfo initInfo{
        .Instance = context->instance,
        .PhysicalDevice = context->physicalDevice,
        .Device = context->device,
        .QueueFamily = context->graphicsQueueIndex,
        .Queue = context->graphicsQueue,
        .DescriptorPool = context->mainDescriptorPool,
        .MinImageCount = 2,
        .ImageCount = imageCount,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = pipelineRenderingInfo,
    };

    ImGui_ImplVulkan_Init(&initInfo);
}

ImGuiPipeline::~ImGuiPipeline() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiPipeline::Begin() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiPipeline::Render(const vk::CommandBuffer cb,
                           const vk::ImageView imageView,
                           const uint32_t width,
                           const uint32_t height) {
    ImGui::Render();

    constexpr vk::ClearValue clearValue{
        .color = std::array<float, 4>({{0.f, 0.f, 0.f, 0.f}}), // useless (only for API)
    };

    vk::RenderingAttachmentInfo colorAttachment{
        .imageView = imageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearValue
    };

    const vk::RenderingInfo renderingInfo{
        .renderArea = {
            .offset = {0, 0},
            .extent = {
                .width = width,
                .height = height
            }
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    cb.beginRendering(renderingInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);

    cb.endRendering();
}
