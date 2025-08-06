#pragma once

#include "Vulkan/Base.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VulkanContext.h"
#include "Raytracer/Context.h"

class ComputePipeline final : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext>& context,
                             const std::shared_ptr<Raytracer::Context>& rtContext);
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

    std::shared_ptr<Raytracer::Context> rtContext;
};
