#pragma once

#include <memory>

#include "Swapchain.h"
#include "VulkanContext.h"

struct Vertex {
    float pos[3];
    float uv[2];
};

class GraphicsPipeline {
public:
    GraphicsPipeline(const std::shared_ptr<VulkanContext>& context, const std::shared_ptr<Swapchain>& swapchain);
    ~GraphicsPipeline();

    void Render(vk::CommandBuffer cb) const;

private:
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateQuad();

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<Swapchain> swapchain;

    vk::PipelineLayout pipelineLayout = nullptr;
    vk::Pipeline pipeline = nullptr;

    vkHelpers::AllocatedBuffer vertexBuffer;
    vkHelpers::AllocatedBuffer indexBuffer;
    uint32_t indexCount;
};
