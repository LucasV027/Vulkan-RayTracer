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
                             const Raytracer& raytracer);

    ~ComputePipeline() override;

    void Dispatch() const;
    void Upload(const Raytracer& raytracer);

    vk::ImageView GetImageView() const { return outputImageView.get(); }

private:
    void CreateCommandPoolAndBuffer();
    void CreateDescriptorSetLayout();
    void CreateDescriptorSet();
    void CreatePipeline();
    void CreateResources();
    void ComputeGroupCount();

    void TransitionForCompute(vk::CommandBuffer cmd) const;
    void TransitionForDisplay(vk::CommandBuffer cmd) const;

private:
    static constexpr uint32_t WORK_GROUP_SIZE_X = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Y = 16;
    uint32_t groupCountX = 1;
    uint32_t groupCountY = 1;
    static constexpr uint32_t GROUP_COUNT_Z = 1;

    // Vulkan
    std::shared_ptr<VulkanContext> context;

    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;

    vk::UniqueDescriptorSet descriptorSet;

    // Cache
    uint32_t width;
    uint32_t height;

    // Resources
    std::unique_ptr<Image> outputImage;
    vk::UniqueImageView outputImageView;
    vk::UniqueSampler sampler;

    std::unique_ptr<Buffer> uniformsBuffer;
};
