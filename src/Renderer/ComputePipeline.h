#pragma once

#include "Raytracer/Raytracer.h"
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
    ~ComputePipeline() override = default;

    void Update(const Raytracer& raytracer);
    void Dispatch(vk::CommandBuffer commandBuffer) const;

    vk::ImageView GetImageView() const { return outputImageView.get(); }
    uint32_t GetFrameIndex() const { return pushData.frameIndex; }

private:
    void CreateDescriptorSetLayout();
    void CreateDescriptorSet();
    void CreatePipeline();
    void CreatePipelineLayout() override;
    void CreateResources();
    void ComputeGroupCount();

    void TransitionForCompute(vk::CommandBuffer cmd) const;
    void TransitionForDisplay(vk::CommandBuffer cmd) const;

private:
    // Constants
    static constexpr uint32_t WORK_GROUP_SIZE_X = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Y = 16;
    static constexpr uint32_t WORK_GROUP_SIZE_Z = 1;

    // Cache
    uint32_t currentWidth;
    uint32_t currentHeight;
    uint32_t groupCountX = 1;
    uint32_t groupCountY = 1;
    uint32_t groupCountZ = 1;

    // GPU Ressources
    std::unique_ptr<Image> outputImage;          // Binding 0
    std::unique_ptr<Buffer> cameraUBO;           // Binding 1
    std::unique_ptr<Buffer> sceneDataUBO;        // Binding 2
    std::unique_ptr<StorageBuffer> meshesSSBO;    // Binding 3
    std::unique_ptr<StorageBuffer> trianglesSSBO; // Binding 4
    std::unique_ptr<StorageBuffer> bvhNodesSSBO;  // Binding 5
    std::unique_ptr<StorageBuffer> spheresSSBO;  // Binding 6
    PushData pushData = {0};

    vk::UniqueImageView outputImageView;
    vk::UniqueDescriptorSet descriptorSet;
};
