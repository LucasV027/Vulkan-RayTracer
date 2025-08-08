cmake_minimum_required(VERSION 3.20)
project(Vulkan-RayTracer)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Application.cpp
        src/Application.h
        src/Core/Log.h
        src/Core/File.h
        src/Core/File.cpp
        src/Window/Window.cpp
        src/Window/Window.h
        src/Vulkan/Base.cpp
        src/Vulkan/Base.h
        src/Renderer/Renderer.cpp
        src/Renderer/Renderer.h
        src/Renderer/ComputePipeline.cpp
        src/Renderer/ComputePipeline.h
        src/Vulkan/VulkanContext.cpp
        src/Vulkan/VulkanContext.h
        src/Renderer/GraphicsPipeline.cpp
        src/Renderer/GraphicsPipeline.h
        src/Renderer/ImGuiPipeline.cpp
        src/Renderer/ImGuiPipeline.h
        src/Vulkan/Swapchain.cpp
        src/Vulkan/Swapchain.h
        src/Vulkan/DescriptorSet.h
        src/Vulkan/DescriptorSet.cpp
        src/Vulkan/DescriptorSet.h
        src/Vulkan/Buffer.cpp
        src/Vulkan/Buffer.h
        src/Vulkan/Image.cpp
        src/Vulkan/Image.h
        src/Vulkan/Pipeline.h
        src/Vulkan/Pipeline.cpp
        src/Raytracer/Raytracer.cpp
        src/Raytracer/Raytracer.h
        src/Raytracer/Data.h
        src/Raytracer/Camera.h
        src/Raytracer/Camera.cpp
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
        glm
)