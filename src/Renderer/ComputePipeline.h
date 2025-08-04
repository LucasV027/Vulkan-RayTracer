#pragma once

#include "Buffer.h"
#include "Image.h"
#include "Pipeline.h"
#include "Vulkan.h"
#include "VulkanContext.h"

class ComputePipeline final : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext>& context);
    ~ComputePipeline() override;

    void Dispatch(vk::CommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z) const;
    vk::ImageView GetView() const { return resultImageView; }

private:
    void CreateDescriptorSet();
    void CreatePipelineLayout();
    void CreatePipeline();

private:
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;

    std::unique_ptr<Buffer> uniformBuffer;
    std::unique_ptr<Image> accumulationImage;
    vk::ImageView accumulationImageView;
    std::unique_ptr<Image> resultImage;
    vk::ImageView resultImageView;
};
