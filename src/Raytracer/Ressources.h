#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace Raytracer {
#define LAYOUT_STD140 alignas(16)

using uint = uint32_t;
using vec2 = std::array<float, 2>;
using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;

struct LAYOUT_STD140 Sphere {
    vec3 center;
    float radius;
};

struct LAYOUT_STD140 Uniforms {
    uint frameIndex;
    float fov;
    vec2 _pad1 = {};

    vec3 cameraPosition;
    float _pad2 = 0.0f;

    vec3 cameraForward;
    float _pad3 = 0.0f;

    vec3 cameraRight;
    float _pad4 = 0.0f;

    vec3 cameraUp;
    float _pad5 = 0.0f;
};

class Ressources {
public:
    Ressources();

    void Update(bool reset);

public:
    static constexpr uint MAX_SPHERES = 100;
    Uniforms uniforms;
    std::vector<Sphere> spheres;
};
}