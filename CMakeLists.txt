cmake_minimum_required(VERSION 3.20)
project(Vulkan-RayTracer)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Application.cpp
        src/Application.h
        src/Renderer/Vulkan.cpp
        src/Renderer/Vulkan.h
        src/Log.h
        src/Utils.h
        src/Utils.cpp
        src/Window.cpp
        src/Window.h
        src/Renderer/Renderer.cpp
        src/Renderer/Renderer.h
        src/Renderer/ComputePipeline.cpp
        src/Renderer/ComputePipeline.h
        src/Renderer/VulkanContext.cpp
        src/Renderer/VulkanContext.h
        src/Renderer/GraphicsPipeline.cpp
        src/Renderer/GraphicsPipeline.h
        src/Renderer/ImGuiPipeline.cpp
        src/Renderer/ImGuiPipeline.h
        src/Renderer/SwapChain.cpp
        src/Renderer/SwapChain.h
        src/Renderer/Pipeline.h
        src/Renderer/DescriptorSet.h
        src/Renderer/DescriptorSet.cpp
        src/Renderer/DescriptorSet.h
        src/Renderer/Buffer.cpp
        src/Renderer/Buffer.h
        src/Renderer/Image.cpp
        src/Renderer/Image.h
        src/Renderer/Pipeline.cpp
        src/Core/RaytracingContext.cpp
        src/Core/RaytracingContext.h
)

include(Dependencies.cmake)

# Includes
target_include_directories(${PROJECT_NAME} PRIVATE
        ${Vulkan_INCLUDE_DIRS}
        ${glfw_SOURCE_DIR}/include
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        src
)

# Sources ImGui (core + backends)
target_sources(${PROJECT_NAME} PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

# Link libs
target_link_libraries(${PROJECT_NAME} PRIVATE
        Vulkan::Vulkan
        glfw
)