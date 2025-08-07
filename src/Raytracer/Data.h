#pragma once

#include <array>
#include <cstdint>

using uint = uint32_t;
using vec2 = std::array<float, 2>;
using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;

#define LAYOUT_STD140 alignas(16)
#define STD140_ASSERT(Type, ExpectedSize) \
static_assert(sizeof(Type) == ExpectedSize, "Size of " #Type " must be " #ExpectedSize " bytes to match std140 layout")
#define PAD(n) float UNIQUE(_pad, __LINE__)[n]
#define UNIQUE(base, line) UNIQUE_NAME(base, line)
#define UNIQUE_NAME(base, line) base##line

constexpr size_t MAX_SPHERES = 50;

struct LAYOUT_STD140 Material {
    vec3 emissionColor;
    float emissionStrength;
    vec3 color;
    PAD(1);
};

struct LAYOUT_STD140 Sphere {
    vec3 pos;
    float rad;
    Material mat;
};

struct LAYOUT_STD140 Scene {
    Sphere spheres[MAX_SPHERES];
    uint32_t count;
    PAD(3);
};

struct LAYOUT_STD140 Uniforms {
    uint frameIndex;
    float fov;
    PAD(2);

    vec3 cameraPosition;
    PAD(1);

    vec3 cameraForward;
    PAD(1);

    vec3 cameraRight;
    PAD(1);

    vec3 cameraUp;
    PAD(1);
};

STD140_ASSERT(Material, 32);
STD140_ASSERT(Sphere, 48);
STD140_ASSERT(Scene, 48 * MAX_SPHERES + 16);
STD140_ASSERT(Uniforms, 80);
