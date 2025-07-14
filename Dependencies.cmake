include(FetchContent)

find_package(Vulkan REQUIRED)

# Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

# Fetch ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

# Fetch ImGui backends (GLFW + Vulkan)
FetchContent_Declare(
        imgui_backends
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG master
)
FetchContent_MakeAvailable(imgui_backends)