#pragma once

#include <cstdint>

#include "Renderer/Vulkan.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/Image.h"
#include "Renderer/Buffer.h"

class RaytracingContext {
public:
    struct Config {
        uint32_t width, height;
        vk::Format outputFormat = vk::Format::eR32G32B32A32Sfloat;
    };

    RaytracingContext(const std::shared_ptr<VulkanContext>& vulkanContext, const Config& config);
    ~RaytracingContext() = default;

    void Update(bool reset = false);
    void Resize(uint32_t width, uint32_t height);

    vk::Image GetAccumulationImage() const { return accumulationImage->GetHandle(); }
    vk::Image GetOutputImage() const { return outputImage->GetHandle(); }

    vk::ImageView GetAccumulationImageView() const { return accumulationImageView.get(); }
    vk::ImageView GetOutputImageView() const { return outputImageView.get(); }

    vk::Sampler GetSampler() const { return sampler.get(); }
    Buffer* GetSceneBuffer() const { return sceneBuffer.get(); }

    uint32_t GetWidth() const { return config.width; }
    uint32_t GetHeight() const { return config.height; }
    uint32_t GetFrameIndex() const { return frameIndex; }

    void TransitionForCompute(vk::CommandBuffer cmd) const;
    void TransitionForDisplay(vk::CommandBuffer cmd) const;
    void CopyResultToAcc(vk::CommandBuffer cmd) const;

private:
    void CreateResources();

private:
    std::shared_ptr<VulkanContext> vulkanContext;
    Config config;

    std::unique_ptr<Image> outputImage;
    vk::UniqueImageView outputImageView;
    vk::UniqueSampler sampler;

    std::unique_ptr<Image> accumulationImage;
    vk::UniqueImageView accumulationImageView;

    std::unique_ptr<Buffer> sceneBuffer;
    uint32_t frameIndex = 0;
};
