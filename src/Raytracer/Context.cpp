#include "Context.h"

namespace Raytracer {
    Context::Context(const std::shared_ptr<VulkanContext>& vulkanContext, const Config& config) :
        vulkanContext(vulkanContext),
        config(config) {
        ressources = std::make_unique<Ressources>();
        CreateResources();
        Update(true);
    }

    void Context::Update(const bool reset) const {
        ressources->Update(reset);
        uniformsBuffer->Update(ressources->uniforms);
    }

    void Context::Resize(const uint32_t width, const uint32_t height) {
        if (config.width == width && config.height == height) return;

        config.width = width;
        config.height = height;

        CreateResources();
        Update(true);
    }

    void Context::TransitionForCompute(const vk::CommandBuffer cmd) const {
        accumulationImage->TransitionLayout(cmd,
                                            vk::ImageLayout::eUndefined,
                                            vk::ImageLayout::eGeneral,
                                            vk::PipelineStageFlagBits2::eTopOfPipe,
                                            vk::PipelineStageFlagBits2::eComputeShader);

        outputImage->TransitionLayout(cmd,
                                      vk::ImageLayout::eUndefined,
                                      vk::ImageLayout::eGeneral,
                                      vk::PipelineStageFlagBits2::eTopOfPipe,
                                      vk::PipelineStageFlagBits2::eComputeShader);
    }

    void Context::TransitionForDisplay(const vk::CommandBuffer cmd) const {
        outputImage->TransitionLayout(cmd,
                                      vk::ImageLayout::eGeneral,
                                      vk::ImageLayout::eShaderReadOnlyOptimal,
                                      vk::PipelineStageFlagBits2::eComputeShader,
                                      vk::PipelineStageFlagBits2::eFragmentShader);
    }

    void Context::CopyResultToAcc(const vk::CommandBuffer cmd) const {
        accumulationImage->TransitionLayout(cmd,
                                            vk::ImageLayout::eGeneral,
                                            vk::ImageLayout::eTransferDstOptimal,
                                            vk::PipelineStageFlagBits2::eComputeShader,
                                            vk::PipelineStageFlagBits2::eTransfer);

        outputImage->TransitionLayout(cmd,
                                      vk::ImageLayout::eGeneral,
                                      vk::ImageLayout::eTransferSrcOptimal,
                                      vk::PipelineStageFlagBits2::eComputeShader,
                                      vk::PipelineStageFlagBits2::eTransfer);

        const vk::ImageCopy copyRegion{
            .srcSubresource = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .dstSubresource = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .extent = {
                outputImage->GetWidth(),
                outputImage->GetHeight(),
                1
            }
        };

        cmd.copyImage(outputImage->GetHandle(), vk::ImageLayout::eTransferSrcOptimal,
                      accumulationImage->GetHandle(), vk::ImageLayout::eTransferDstOptimal,
                      1, &copyRegion);

        accumulationImage->TransitionLayout(cmd,
                                            vk::ImageLayout::eTransferDstOptimal,
                                            vk::ImageLayout::eGeneral,
                                            vk::PipelineStageFlagBits2::eTransfer,
                                            vk::PipelineStageFlagBits2::eComputeShader);

        outputImage->TransitionLayout(cmd,
                                      vk::ImageLayout::eTransferSrcOptimal,
                                      vk::ImageLayout::eGeneral,
                                      vk::PipelineStageFlagBits2::eTransfer,
                                      vk::PipelineStageFlagBits2::eComputeShader);
    }

    void Context::CreateResources() {
        outputImage = std::make_unique<Image>(vulkanContext,
                                              config.width,
                                              config.height,
                                              vk::Format::eR32G32B32A32Sfloat,
                                              vk::ImageUsageFlagBits::eStorage |
                                              vk::ImageUsageFlagBits::eSampled |
                                              vk::ImageUsageFlagBits::eTransferSrc);

        outputImageView = outputImage->CreateView();

        accumulationImage = std::make_unique<Image>(vulkanContext,
                                                    config.width,
                                                    config.height,
                                                    vk::Format::eR32G32B32A32Sfloat,
                                                    vk::ImageUsageFlagBits::eStorage |
                                                    vk::ImageUsageFlagBits::eSampled |
                                                    vk::ImageUsageFlagBits::eTransferDst);

        accumulationImageView = accumulationImage->CreateView();

        uniformsBuffer = std::make_unique<Buffer>(vulkanContext, sizeof(Uniforms),
                                                  vk::BufferUsageFlagBits::eUniformBuffer);

        constexpr vk::SamplerCreateInfo samplerInfo{
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .mipLodBias = 0.0f,
            .anisotropyEnable = vk::False,
            .maxAnisotropy = 1.0f,
            .compareEnable = vk::False,
            .compareOp = vk::CompareOp::eAlways,
            .minLod = 0.0f,
            .maxLod = vk::LodClampNone,
            .borderColor = vk::BorderColor::eIntOpaqueBlack,
            .unnormalizedCoordinates = vk::False,
        };

        sampler = vulkanContext->device.createSamplerUnique(samplerInfo);
    }
}
