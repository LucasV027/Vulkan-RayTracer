#pragma once

#include <memory>

#include "Buffer.h"
#include "Image.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "VulkanContext.h"
#include "Core/RaytracingContext.h"

struct Vertex {
    float pos[3];
    float uv[2];
};

class GraphicsPipeline final : public Pipeline {
public:
    GraphicsPipeline(const std::shared_ptr<VulkanContext>& context,
                     const std::shared_ptr<Swapchain>& swapchain,
                     const std::shared_ptr<RaytracingContext>& rtContext);

    ~GraphicsPipeline() override = default;

    void Record(vk::CommandBuffer cb) const override;

private:
    void CreateDescriptorSet();
    void CreatePipeline();
    void CreateQuad();

private:
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<RaytracingContext> rtContext;

    vk::DescriptorSet descriptorSet;

    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexBuffer;
    uint32_t indexCount = 0u;
};
