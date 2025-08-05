#include "Ressources.h"

#include <cmath>

namespace Raytracer {
    Ressources::Ressources() {
        uniforms = {
            .frameIndex = 0,
            .fov = 90.f * (M_PI / 180),
            .cameraPosition = {0.f, 0.f, 0.f},
            .cameraForward = {0.f, 0.f, -1.f},
            .cameraRight = {1.f, 0.f, 0.f},
            .cameraUp = {0.f, 1.f, 0.f},
        };

        spheres.reserve(MAX_SPHERES);
    }

    void Ressources::Update(const bool reset) {
        if (reset) uniforms.frameIndex = 0;
        uniforms.frameIndex++;
    }
}
