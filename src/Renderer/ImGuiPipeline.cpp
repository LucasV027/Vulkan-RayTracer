#include "ImGuiPipeline.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

ImGuiPipeline::ImGuiPipeline(const std::shared_ptr<VulkanContext>& context,
                             const std::shared_ptr<Window>& window,
                             const std::shared_ptr<Swapchain>& swapchain)
    : Pipeline(context),
      window(window),
      swapchain(swapchain) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    // Init GLFW
    ImGui_ImplGlfw_InitForVulkan(window->Handle(), true);

    // Init Vulkan backend
    auto format = swapchain->GetFormat();
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
        .ImageCount = swapchain->GetImageCount(),
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

void ImGuiPipeline::Begin() const {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    frame = true;
}

void ImGuiPipeline::End() const {
    if (frame) ImGui::EndFrame();
}

void ImGuiPipeline::Record(const vk::CommandBuffer cb) const {
    if (!frame) return;

    const auto& fc = swapchain->GetCurrentFrameContext();

    ImGui::Render();

    constexpr vk::ClearValue clearValue{
        .color = std::array<float, 4>({{0.f, 0.f, 0.f, 0.f}}), // useless (only for API)
    };

    vk::RenderingAttachmentInfo colorAttachment{
        .imageView = swapchain->GetImageViews()[fc.index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearValue
    };

    const vk::RenderingInfo renderingInfo{
        .renderArea = {
            .offset = {0, 0},
            .extent = swapchain->GetExtent(),
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    cb.beginRendering(renderingInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);

    cb.endRendering();

    frame = false;
}
