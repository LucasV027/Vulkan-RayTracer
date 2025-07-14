#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <cassert>

GLFWwindow* window;

bool GLFW_Init() {
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // No resize yet !

    window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    assert(GLFW_Init());

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
