#pragma once

#include <memory>

#include "Vulkan/Base.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/VulkanContext.h"
#include "Raytracer/Context.h"

struct Vertex {
    float pos[3];
    float uv[2];
};

class GraphicsPipeline final : public Pipeline {
public:
    GraphicsPipeline(const std::shared_ptr<VulkanContext>& context,
                     const std::shared_ptr<Swapchain>& swapchain,
                     const std::shared_ptr<Raytracer::Context>& rtContext);

    ~GraphicsPipeline() override = default;

    void Record(vk::CommandBuffer cb) const override;

    void Resize();

private:
    void CreateDescriptorSetLayout();
    void CreateDescriptorSet();
    void CreatePipeline();
    void CreateQuad();

private:
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<Raytracer::Context> rtContext;

    vk::UniqueDescriptorSet descriptorSet;

    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexBuffer;
    uint32_t indexCount = 0u;
};
