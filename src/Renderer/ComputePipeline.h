#pragma once

#include "Raytracer/Raytracer.h"
#include "Vulkan/Base.h"
#include "Vulkan/Image.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VulkanContext.h"

class ComputePipeline final : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext>& context,
                             const std::shared_ptr<Raytracer>& raytracer);

    ~ComputePipeline() override = default;

    void Record(vk::CommandBuffer cb) const override;

    void Resize();
    void Update() const;

    vk::ImageView GetImageView() const { return outputImageView.get(); }

private:
    void CreateDescriptorSetLayout();
    void CreateDescriptorSet();
    void CreatePipeline();
    void CreateResources();

    void TransitionForCompute(vk::CommandBuffer cmd) const;
    void TransitionForDisplay(vk::CommandBuffer cmd) const;

private:
    static constexpr uint32_t WORK_GROUP_SIZE_X = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Y = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Z = 1;

    vk::UniqueDescriptorSet descriptorSet;
    std::shared_ptr<Raytracer> raytracer;

    // Resources
    std::unique_ptr<Image> outputImage;
    vk::UniqueImageView outputImageView;
    vk::UniqueSampler sampler;

    std::unique_ptr<Buffer> uniformsBuffer;
};
