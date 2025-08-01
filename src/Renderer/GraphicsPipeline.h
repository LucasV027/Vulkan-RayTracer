#pragma once

#include <memory>

#include "Buffer.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "VulkanContext.h"

struct Vertex {
    float pos[3];
    float uv[2];
};

class GraphicsPipeline final : public Pipeline {
public:
    GraphicsPipeline(const std::shared_ptr<VulkanContext>& context,
                     const std::shared_ptr<Swapchain>& swapchain);

    ~GraphicsPipeline() override = default;

    void Render(vk::CommandBuffer cb) const;

private:
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateQuad();

private:
    std::shared_ptr<Swapchain> swapchain;

    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexBuffer;
    uint32_t indexCount = 0u;
};
