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

    void Resize();

private:
    void CreateDescriptorSetLayout();
    void CreateDescriptorSet();
    void CreatePipeline();

private:
    static constexpr uint32_t WORK_GROUP_SIZE_X = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Y = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Z = 1;

    vk::UniqueDescriptorSet descriptorSet;

    std::shared_ptr<RaytracingContext> rtContext;
};
