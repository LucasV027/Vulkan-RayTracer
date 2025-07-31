#pragma once

#include "Vulkan.h"
#include "VulkanContext.h"

class Pipeline {
public:
    explicit Pipeline(const std::shared_ptr<VulkanContext>& context) : context(context) {}

    virtual ~Pipeline() {
        if (context && context->device) {
            if (pipeline) context->device.destroyPipeline(pipeline);
            if (pipelineLayout) context->device.destroyPipelineLayout(pipelineLayout);
        }
    };

protected:
    std::shared_ptr<VulkanContext> context;

    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
};

