#include "GraphicsPipeline.h"

#include "Vulkan/DescriptorSet.h"

GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<VulkanContext>& context,
                                   const std::shared_ptr<Swapchain>& swapchain) :
    Pipeline(context),
    swapchain(swapchain) {
    CreateSampler();
    CreateDescriptorSetLayout();
    Pipeline::CreatePipelineLayout();
    CreatePipeline();
}

void GraphicsPipeline::Record(const vk::CommandBuffer cb) const {
    const auto& fc = swapchain->GetCurrentFrameContext();

    constexpr vk::ClearValue clearValue{
        .color = std::array<float, 4>({{0.01f, 0.01f, 0.033f, 1.0f}}),
    };

    vk::RenderingAttachmentInfo colorAttachment{
        .imageView = swapchain->GetImageViews()[fc.index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearValue
    };

    const auto extent = swapchain->GetExtent();
    const vk::RenderingInfo renderingInfo{
        .renderArea = {
            .offset = {0, 0},
            .extent = extent,
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    const vk::Viewport vp{
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    const vk::Rect2D scissor{
        .extent = extent,
    };

    cb.beginRendering(renderingInfo);

    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

    cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet.get(), {});

    cb.setViewport(0, vp);
    cb.setScissor(0, scissor);

    cb.setCullMode(vk::CullModeFlagBits::eNone);
    cb.setFrontFace(vk::FrontFace::eClockwise);
    cb.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);

    cb.draw(6, 1, 0, 0);

    cb.endRendering();
}

void GraphicsPipeline::Resize() {
    CreateDescriptorSet();
}

void GraphicsPipeline::SetImageView(const vk::ImageView newImageView) {
    if (imageView != newImageView) {
        imageView = newImageView;
        CreateDescriptorSet();
    }
}

void GraphicsPipeline::CreateDescriptorSetLayout() {
    DescriptorSetLayoutBuilder layoutBuilder;
    layoutBuilder.AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                 .AddTo(vulkanContext->device, descriptorSetLayouts);
}

void GraphicsPipeline::CreateDescriptorSet() {
    descriptorSet = std::move(AllocateDescriptorSets()[0]);

    DescriptorSetWriter writer;
    constexpr auto layout = vk::ImageLayout::eShaderReadOnlyOptimal;
    writer.WriteCombinedImageSampler(0, sampler.get(), imageView, layout)
          .Update(vulkanContext->device, descriptorSet.get());
}

void GraphicsPipeline::CreatePipeline() {
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    vk::PipelineRasterizationStateCreateInfo raster{
        .polygonMode = vk::PolygonMode::eFill,
        .lineWidth = 1.0f
    };

    // Specify that these states will be dynamic, i.e. not part of pipeline state object.
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
        vk::DynamicState::eCullMode,
        vk::DynamicState::eFrontFace,
        vk::DynamicState::ePrimitiveTopology
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    vk::PipelineColorBlendAttachmentState blendAttachment{
        .colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo blend{
        .attachmentCount = 1,
        .pAttachments = &blendAttachment
    };

    vk::PipelineViewportStateCreateInfo viewport{
        .viewportCount = 1,
        .scissorCount = 1
    };

    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthCompareOp = vk::CompareOp::eAlways
    };

    vk::PipelineMultisampleStateCreateInfo multisample{
        .rasterizationSamples = vk::SampleCountFlagBits::e1
    };

    auto vertShaderModule = vkHelpers::CreateShaderModule(vulkanContext->device, "../shaders/main.vert.spv");
    auto fragShaderModule = vkHelpers::CreateShaderModule(vulkanContext->device, "../shaders/main.frag.spv");

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {
        {
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = vertShaderModule.get(),
                .pName = "main"
            },
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = fragShaderModule.get(),
                .pName = "main"
            }
        }
    };

    auto format = swapchain->GetFormat();
    vk::PipelineRenderingCreateInfo pipelineRenderingInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &format,
    };

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
        .pNext = &pipelineRenderingInfo,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewport,
        .pRasterizationState = &raster,
        .pMultisampleState = &multisample,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &blend,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = nullptr, // Since we are using dynamic rendering this will set as null
        .subpass = 0,
    };

    vk::Result result;
    std::tie(result, pipeline) = vulkanContext->device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
    if (result != vk::Result::eSuccess) throw std::runtime_error("failed to create graphics pipeline");
}

void GraphicsPipeline::CreateSampler() {
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

