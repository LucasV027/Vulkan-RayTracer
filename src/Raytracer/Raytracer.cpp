#include "Raytracer.h"

Raytracer::Raytracer(const uint32_t width, const uint32_t height) : width(width),
                                                                    height(height),
                                                                    scene({}) {
    camera = std::make_shared<Camera>();
    SetAllDirty();
}

void Raytracer::Update(const uint32_t newWidth, const uint32_t newHeight) {
    if (newWidth != width || newHeight != height) {
        width = newWidth;
        height = newHeight;
    }
}
