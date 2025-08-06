#pragma once

#include <cstdint>
#include <cmath>

#include "Data.h"

class Raytracer {
public:
    Raytracer(const uint32_t width, const uint32_t height) : width(width), height(height) {
        data = {
            .frameIndex = 0,
            .fov = 90.f * (M_PI / 180),
            .cameraPosition = {0.f, 0.f, 0.f},
            .cameraForward = {0.f, 0.f, -1.f},
            .cameraRight = {1.f, 0.f, 0.f},
            .cameraUp = {0.f, 1.f, 0.f},
        };
    }

    void Update() {
        data.frameIndex++;
    }

    void Resize(const uint32_t newWidth, const uint32_t newHeight) {
        width = newWidth;
        height = newHeight;
        data.frameIndex = 0;
    }

    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }
    const CPU::Data& GetData() const { return data; }
    uint32_t GetFrameIndex() const { return data.frameIndex; }

private:
    uint32_t width;
    uint32_t height;
    CPU::Data data;
};
