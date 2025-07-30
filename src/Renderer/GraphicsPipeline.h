#pragma once

#include <memory>

#include "Swapchain.h"
#include "VulkanContext.h"

class GraphicsPipeline {
public:
    GraphicsPipeline(const std::shared_ptr<VulkanContext>& context, const std::shared_ptr<Swapchain>& swapchain);
    ~GraphicsPipeline();

    void Render(vk::CommandBuffer cb) const;

private:
    void CreateGraphicsPipeline();
    void CreateVertexBuffer();

    const std::vector<float> QUAD = {
        -1.f, -1.f, 1.f, -1.f, 1.f, 1.f,
        -1.f, -1.f, 1.f, 1.f, -1.f, 1.f,
    };

private:
    std::shared_ptr<VulkanContext> context;
    std::shared_ptr<Swapchain> swapchain;

    vk::PipelineLayout pipelineLayout = nullptr;
    vk::Pipeline pipeline = nullptr;

    vk::Buffer vertexBuffer = nullptr;
    vk::DeviceMemory vertexBufferMemory = nullptr;
};
