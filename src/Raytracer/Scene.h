#pragma once

#include <glm/glm.hpp>

#include "Core/Base.h"

constexpr size_t MAX_SPHERES = 50;
constexpr float MAX_POS = 100.f;
constexpr float MIN_POS = -100.f;
constexpr float MIN_SPHERE_RADIUS = 1.0f;
constexpr float MAX_SPHERE_RADIUS = 100.0f;
constexpr float MAX_EMISSION_STRENGTH = 100.0f;
constexpr float MIN_EMISSION_STRENGTH = 0.0f;

struct LAYOUT_STD140 Material {
    glm::vec3 emissionColor;
    float emissionStrength;
    glm::vec3 color;
    PAD(1);
};

struct LAYOUT_STD140 Sphere {
    glm::vec3 pos;
    float rad;
    Material mat;
};

struct LAYOUT_STD140 SceneData {
    Sphere spheres[MAX_SPHERES];
    uint32_t count;
    PAD(3);
};

class Scene {
public:
    Scene();
    ~Scene() = default;

    void DrawUI();

    const SceneData& GetData() const { return sceneData; }
    bool NeedsUpdate() const { return needsUpdate; }

    void ResetUpdate() const { needsUpdate = false; }

private:
    SceneData sceneData;
    mutable bool needsUpdate = true;
};
