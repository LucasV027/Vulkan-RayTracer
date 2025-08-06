#pragma once

#include <cstdint>

#include "Ressources.h"
#include "Vulkan/Base.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/Image.h"
#include "Vulkan/Buffer.h"

namespace Raytracer {
    class Context {
    public:
        struct Config {
            uint32_t width;
            uint32_t height;
            vk::Format outputFormat = vk::Format::eR32G32B32A32Sfloat;
        };

        Context(const std::shared_ptr<VulkanContext>& vulkanContext, const Config& config);
        ~Context() = default;

        void Update(bool reset = false) const;
        void Resize(uint32_t width, uint32_t height);

        vk::Image GetAccumulationImage() const { return accumulationImage->GetHandle(); }
        vk::Image GetOutputImage() const { return outputImage->GetHandle(); }

        vk::ImageView GetAccumulationImageView() const { return accumulationImageView.get(); }
        vk::ImageView GetOutputImageView() const { return outputImageView.get(); }

        vk::Sampler GetSampler() const { return sampler.get(); }
        Buffer* GetSceneBuffer() const { return uniformsBuffer.get(); }

        uint32_t GetWidth() const { return config.width; }
        uint32_t GetHeight() const { return config.height; }
        uint32_t GetFrameIndex() const { return ressources->uniforms.frameIndex; }

        void TransitionForCompute(vk::CommandBuffer cmd) const;
        void TransitionForDisplay(vk::CommandBuffer cmd) const;
        void CopyResultToAcc(vk::CommandBuffer cmd) const;

    private:
        void CreateResources();

    private:
        std::shared_ptr<VulkanContext> vulkanContext;
        std::unique_ptr<Ressources> ressources;
        Config config;

        std::unique_ptr<Image> outputImage;
        vk::UniqueImageView outputImageView;
        vk::UniqueSampler sampler;

        std::unique_ptr<Image> accumulationImage;
        vk::UniqueImageView accumulationImageView;

        std::unique_ptr<Buffer> uniformsBuffer;
    };
}
