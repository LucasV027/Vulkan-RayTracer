#pragma once

#include "Raytracer/Scene.h"

namespace UI {
    constexpr float MAX_POS = 100.f;
    constexpr float MIN_POS = -100.f;
    constexpr float MIN_SPHERE_RADIUS = 1.0f;
    constexpr float MAX_SPHERE_RADIUS = 100.0f;
    constexpr float MAX_EMISSION_STRENGTH = 100.0f;
    constexpr float MIN_EMISSION_STRENGTH = 0.0f;
    constexpr float MIN_SMOOTHNESS = 0.0f;
    constexpr float MAX_SMOOTHNESS = 1.0f;

    bool DrawMaterial(Material& mat);
    bool DrawSphere(Sphere& sphere);
    bool DrawTransform(glm::mat4& transform);
    bool DrawMesh(Mesh& mesh);
    void DrawScene(Scene& scene);
}
