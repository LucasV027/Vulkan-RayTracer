#pragma once

#include "Buffer.h"
#include "Image.h"
#include "Pipeline.h"
#include "Vulkan.h"
#include "VulkanContext.h"

class ComputePipeline final : public Pipeline {
public:
    explicit ComputePipeline(const std::shared_ptr<VulkanContext>& context,
                             const std::shared_ptr<Image>& resultImage);
    ~ComputePipeline() override;

    void Record(vk::CommandBuffer cb) const override;

private:
    void Dispatch(vk::CommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z) const;

    void CreateDescriptorSet();
    void CreatePipeline();

private:
    vk::DescriptorSet descriptorSet;

    std::unique_ptr<Buffer> uniformBuffer;
    std::unique_ptr<Image> accumulationImage;
    vk::UniqueImageView accumulationImageView;

    std::shared_ptr<Image> resultImage;
    vk::UniqueImageView resultImageView;
};
