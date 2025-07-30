#include "GraphicsPipeline.h"

GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<VulkanContext>& context,
                                   const std::shared_ptr<Swapchain>& swapchain) : context(context),
    swapchain(swapchain) {
    CreateGraphicsPipeline();
    CreateVertexBuffer();
}

GraphicsPipeline::~GraphicsPipeline() {
    if (pipeline) context->device.destroyPipeline(pipeline);
    if (pipelineLayout) context->device.destroyPipelineLayout(pipelineLayout);

    if (vertexBuffer) context->device.destroyBuffer(vertexBuffer);
    if (vertexBufferMemory) context->device.freeMemory(vertexBufferMemory);
}

void GraphicsPipeline::Render(const vk::CommandBuffer cb) const {
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

    cb.bindVertexBuffers(0, vertexBuffer, {0});
    cb.draw(QUAD.size(), 1, 0, 0);

    cb.endRendering();
}

void GraphicsPipeline::CreateGraphicsPipeline() {
    pipelineLayout = context->device.createPipelineLayout({});

    vk::VertexInputBindingDescription bindingDescription{
        .binding = 0,
        .stride = 2 * sizeof(float),
        .inputRate = vk::VertexInputRate::eVertex
    };

    // Define the vertex input attribute descriptions
    std::array<vk::VertexInputAttributeDescription, 1> attributeDescriptions = {
        {
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = 0
            },
        }
    };

    // Create the vertex input state
    vk::PipelineVertexInputStateCreateInfo vertexInput{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };


    // Specify we will use triangle lists to draw geometry.
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList
    };

    // Specify rasterization state.
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

    auto vertShaderModule = vkHelpers::CreateShaderModule(context->device, "../shaders/main.vert.spv");
    auto fragShaderModule = vkHelpers::CreateShaderModule(context->device, "../shaders/main.frag.spv");

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
        .layout = pipelineLayout,
        // We need to specify the pipeline layout description up front as well.
        .renderPass = nullptr, // Since we are using dynamic rendering this will set as null
        .subpass = 0,
    };

    vk::Result result;
    std::tie(result, pipeline) = context->device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
    if (result != vk::Result::eSuccess) throw std::runtime_error("failed to create graphics pipeline");
}

void GraphicsPipeline::CreateVertexBuffer() {
    const vk::DeviceSize bufferSize = sizeof(QUAD[0]) * QUAD.size();

    const vk::BufferCreateInfo vertexBufferCreateInfo{
        .size = bufferSize,
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };

    vertexBuffer = context->device.createBuffer(vertexBufferCreateInfo);

    const vk::MemoryRequirements memoryRequirements = context->device.getBufferMemoryRequirements(vertexBuffer);

    const vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex =
        vkHelpers::FindMemoryType(context->physicalDevice, memoryRequirements.memoryTypeBits,
                                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    };

    vertexBufferMemory = context->device.allocateMemory(allocInfo);

    context->device.bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);

    void* data = context->device.mapMemory(vertexBufferMemory, 0, bufferSize);
    memcpy(data, QUAD.data(), bufferSize);
    context->device.unmapMemory(vertexBufferMemory);
}
