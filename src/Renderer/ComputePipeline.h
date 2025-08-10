#pragma once

#include "Raytracer/Camera.h"
#include "Vulkan/Base.h"
#include "Vulkan/Image.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VulkanContext.h"

struct PushData {
    uint32_t frameIndex;
};

class ComputePipeline final : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext>& context);

    ~ComputePipeline() override;

    void Update(const Camera& camera, uint32_t width, uint32_t height);
    void Dispatch() const;

    vk::ImageView GetImageView() const { return outputImageView.get(); }
    uint32_t GetFrameIndex() const { return pushData.frameIndex; }

private:
    void CreateCommandPoolAndBuffer();
    void CreateDescriptorSetLayout();
    void CreateDescriptorSet();
    void CreatePipeline();
    void CreatePipelineLayout() override;
    void CreateResources();
    void ComputeGroupCount();

    void TransitionForCompute(vk::CommandBuffer cmd) const;
    void TransitionForDisplay(vk::CommandBuffer cmd) const;

private:
    static constexpr uint32_t WORK_GROUP_SIZE_X = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Y = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Z = 1;
    uint32_t groupCountX = 1;
    uint32_t groupCountY = 1;
    uint32_t groupCountZ = 1;

    // Vulkan
    std::shared_ptr<VulkanContext> context;

    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;

    vk::UniqueDescriptorSet descriptorSet;

    // Cache
    uint32_t currentWidth;
    uint32_t currentHeight;

    // Resources
    std::unique_ptr<Buffer> cameraBuffer; // Binding 0
    std::unique_ptr<Image> outputImage;   // Binding 1
    vk::UniqueImageView outputImageView;
    PushData pushData = {0};
};
