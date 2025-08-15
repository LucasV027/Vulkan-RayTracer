cmake_minimum_required(VERSION 3.24)
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
        src/Raytracer/Camera.cpp
        src/Raytracer/Camera.h
        src/Raytracer/Scene.cpp
        src/Raytracer/Scene.h
        src/Controller/CameraController.cpp
        src/Controller/CameraController.h
        src/Raytracer/Raytracer.cpp
        src/Raytracer/Raytracer.h
        src/Extern/objload.h
        src/UI/SceneUI.cpp
        src/UI/SceneUI.h
        src/UI/BaseUI.h
        src/UI/Base.cpp
        src/UI/RaytracerUI.cpp
        src/UI/RaytracerUI.h
        src/UI/CameraUI.cpp
        src/UI/CameraUI.h
        src/Raytracer/ComputeData.h
        src/Core/Math.cpp
        src/Core/Math.h
)

# NOTE: This hardcoded path works only for local development builds.
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets/")
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH=\"${ASSETS_DIR}\")


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
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

# Link libs
target_link_libraries(${PROJECT_NAME} PRIVATE
        Vulkan::Vulkan
        glfw
        glm
)