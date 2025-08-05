#include "Application.h"

int main(int argc, char** argv) {
    const auto app = Application("Vulkan-RayTracer", 800, 600);
    app.Run();
}
