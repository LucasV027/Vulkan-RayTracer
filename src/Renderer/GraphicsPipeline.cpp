#include "GraphicsPipeline.h"

#include "DescriptorSet.h"

GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<VulkanContext>& context,
                                   const std::shared_ptr<Swapchain>& swapchain,
                                   const std::shared_ptr<RaytracingContext>& rtContext) :
    Pipeline(context),
    swapchain(swapchain),
    rtContext(rtContext) {
    CreateDescriptorSetLayout();
    CreateDescriptorSet();
    Pipeline::CreatePipelineLayout();
    CreatePipeline();
    CreateQuad();
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

    cb.beginRendering(renderingInfo);

    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

    cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet.get(), {});

    const vk::Viewport vp{
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    const vk::Rect2D scissor{
        .extent = extent,
    };

    cb.setViewport(0, vp);
    cb.setScissor(0, scissor);

    cb.setCullMode(vk::CullModeFlagBits::eNone);
    cb.setFrontFace(vk::FrontFace::eClockwise);
    cb.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);

    constexpr vk::DeviceSize offsets[] = {0};
    const auto vbo = vertexBuffer->GetHandle();
    cb.bindVertexBuffers(0, 1, &vbo, offsets);
    cb.bindIndexBuffer(indexBuffer->GetHandle(), 0, vk::IndexType::eUint32);
    cb.drawIndexed(indexCount, 1, 0, 0, 0);

    cb.endRendering();
}

void GraphicsPipeline::Resize() {
    CreateDescriptorSet();
}

void GraphicsPipeline::CreateDescriptorSetLayout() {
    DescriptorSetLayoutBuilder layoutBuilder;
    layoutBuilder.AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                 .AddTo(vulkanContext->device, descriptorSetLayouts);
}

void GraphicsPipeline::CreateDescriptorSet() {
    descriptorSet = std::move(AllocateDescriptorSets()[0]);

    DescriptorSetWriter writer;
    writer.WriteCombinedImageSampler(0, rtContext->GetSampler(), rtContext->GetOutputImageView(),
                                     vk::ImageLayout::eShaderReadOnlyOptimal)
          .Update(vulkanContext->device, descriptorSet.get());
}

void GraphicsPipeline::CreatePipeline() {
    vk::VertexInputBindingDescription vertexInputBindings{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex
    };

    std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributes{
        {
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = 0
            },
            {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = sizeof(Vertex::pos),
            }
        }
    };

    vk::PipelineVertexInputStateCreateInfo vertexInput{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexInputBindings,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size()),
        .pVertexAttributeDescriptions = vertexInputAttributes.data()
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList
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

    // Our attachment will write to all color channels, but no blending is enabled.
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

    // We will have one viewport and scissor box.
    vk::PipelineViewportStateCreateInfo viewport{
        .viewportCount = 1,
        .scissorCount = 1
    };

    // Disable all depth testing.
    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthCompareOp = vk::CompareOp::eAlways
    };

    // No multisampling.
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

    // Pipeline rendering info (for dynamic rendering).
    auto format = swapchain->GetFormat();
    vk::PipelineRenderingCreateInfo pipelineRenderingInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &format,
    };

    // Create the graphics pipeline.
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
        .pNext = &pipelineRenderingInfo,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewport,
        .pRasterizationState = &raster,
        .pMultisampleState = &multisample,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &blend,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = pipelineLayout, // We need to specify the pipeline layout description up front as well.
        .renderPass = nullptr,    // Since we are using dynamic rendering this will set as null
        .subpass = 0,
    };

    vk::Result result;
    std::tie(result, pipeline) = vulkanContext->device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
    if (result != vk::Result::eSuccess) throw std::runtime_error("failed to create graphics pipeline");
}

void GraphicsPipeline::CreateQuad() {
    static const std::vector<Vertex> QUAD_UV = {
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}
    };

    static const std::vector<uint32_t> QUAD_INDICES = {0, 1, 2, 2, 3, 0};

    indexCount = static_cast<uint32_t>(QUAD_INDICES.size());
    vertexBuffer = std::make_unique<Buffer>(vulkanContext, QUAD_UV, vk::BufferUsageFlagBits::eVertexBuffer);
    indexBuffer = std::make_unique<Buffer>(vulkanContext, QUAD_INDICES, vk::BufferUsageFlagBits::eIndexBuffer);
}

