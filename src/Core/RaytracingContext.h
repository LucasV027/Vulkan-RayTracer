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

    vk::Image GetOutputImage() const { return outputImage->GetHandle(); }
    vk::Sampler GetSampler() const { return sampler.get(); }
    vk::ImageView GetOutputImageView() const { return outputImageView.get(); }
    Buffer* GetSceneBuffer() const { return sceneBuffer.get(); }
    uint32_t GetWidth() const { return config.width; }
    uint32_t GetHeight() const { return config.height; }

    void TransitionForCompute(vk::CommandBuffer cmd) const;
    void TransitionForDisplay(vk::CommandBuffer cmd) const;

private:
    void CreateResources();

private:
    std::shared_ptr<VulkanContext> vulkanContext;
    Config config;

    std::unique_ptr<Image> outputImage;
    vk::UniqueImageView outputImageView;
    std::unique_ptr<Buffer> sceneBuffer;
    vk::UniqueSampler sampler;
};
