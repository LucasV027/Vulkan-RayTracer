#pragma once

#include "Pipeline.h"
#include "Vulkan.h"
#include "VulkanContext.h"
#include "Core/RaytracingContext.h"

class ComputePipeline final : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext>& context,
                             const std::shared_ptr<RaytracingContext>& rtContext);
    ~ComputePipeline() override = default;

    void Record(vk::CommandBuffer cb) const override;

private:
    void CreateDescriptorSet();
    void CreatePipeline();

private:
    static constexpr uint32_t workgroupSizeX = 16;
    static constexpr uint32_t workgroupSizeY = 16;
    static constexpr uint32_t workgroupSizeZ = 1;

    vk::DescriptorSet descriptorSet;

    std::shared_ptr<RaytracingContext> rtContext;
};
