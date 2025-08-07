#pragma once

#include <cstdint>

#include "Data.h"

class Raytracer {
public:
    Raytracer(uint32_t width, uint32_t height);

    void Update();
    void RenderUI();

    void OnResize(uint32_t newWidth, uint32_t newHeight);

    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }
    uint32_t GetFrameIndex() const { return uniforms.frameIndex; }

    const Uniforms& GetUniforms() const { return uniforms; }

private:
    uint32_t width;
    uint32_t height;
    Uniforms uniforms{};
};
