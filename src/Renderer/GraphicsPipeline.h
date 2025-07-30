#pragma once

#include <memory>

#include "VulkanContext.h"

class GraphicsPipeline {
public:
    GraphicsPipeline(const std::shared_ptr<VulkanContext>& context, vk::Format format);
    ~GraphicsPipeline();

    void Render(vk::CommandBuffer cb, vk::ImageView imageView, uint32_t width, uint32_t height) const;

private:
    void CreateGraphicsPipeline(vk::Format format);
    void CreateVertexBuffer();

    const std::vector<float> QUAD = {
        -1.f, -1.f, 1.f, -1.f, 1.f, 1.f,
        -1.f, -1.f, 1.f, 1.f, -1.f, 1.f,
    };

private:
    std::shared_ptr<VulkanContext> context;

    vk::PipelineLayout pipelineLayout = nullptr;
    vk::Pipeline pipeline = nullptr;

    vk::Buffer vertexBuffer = nullptr;
    vk::DeviceMemory vertexBufferMemory = nullptr;
};
