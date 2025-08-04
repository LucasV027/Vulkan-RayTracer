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

    void Record(vk::CommandBuffer cb) const override;

    vk::ImageView GetView() const { return resultImageView; }

private:
    void Dispatch(vk::CommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z) const;

    void CreateDescriptorSet();
    void CreatePipeline();

private:
    vk::DescriptorSet descriptorSet;

    std::unique_ptr<Buffer> uniformBuffer;
    std::unique_ptr<Image> accumulationImage;
    vk::ImageView accumulationImageView;
    std::unique_ptr<Image> resultImage;
    vk::ImageView resultImageView;
};
