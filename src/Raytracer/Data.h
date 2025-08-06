#pragma once

#include <array>
#include <cstdint>

using uint = uint32_t;
using vec2 = std::array<float, 2>;
using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;

namespace GPU {
#define LAYOUT_STD140 alignas(16)

    struct LAYOUT_STD140 Sphere {
        vec3 center;
        float radius;
    };

    struct LAYOUT_STD140 Data {
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
}

namespace CPU {
    struct Data {
        uint frameIndex;
        float fov;
        vec3 cameraPosition;
        vec3 cameraForward;
        vec3 cameraRight;
        vec3 cameraUp;
    };

    inline GPU::Data ToGPU(const Data& data) {
        return GPU::Data{
            .frameIndex = data.frameIndex,
            .fov = data.fov,
            .cameraPosition = data.cameraPosition,
            .cameraForward = data.cameraForward,
            .cameraRight = data.cameraRight,
            .cameraUp = data.cameraUp,
        };
    }
}
