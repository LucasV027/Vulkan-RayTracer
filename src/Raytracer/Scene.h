#pragma once

#include <glm/glm.hpp>

#include "Core/Base.h"

constexpr size_t MAX_SPHERES = 20;
constexpr float MAX_POS = 100.f;
constexpr float MIN_POS = -100.f;
constexpr float MIN_SPHERE_RADIUS = 1.0f;
constexpr float MAX_SPHERE_RADIUS = 100.0f;
constexpr float MAX_EMISSION_STRENGTH = 100.0f;
constexpr float MIN_EMISSION_STRENGTH = 0.0f;
constexpr float MIN_SMOOTHNESS = 0.0f;
constexpr float MAX_SMOOTHNESS = 1.0f;

struct LAYOUT_STD140 Material {
    glm::vec3 color;
    float smoothness;
    glm::vec3 emissionColor;
    float emissionStrength;

    bool DrawUI();
};

struct LAYOUT_STD140 Sphere {
    glm::vec3 pos;
    float rad;
    Material mat;

    bool DrawUI();
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

    void AddSphere();
    void RemoveSphere(uint32_t idx);

    bool Full() const { return sceneData.count == MAX_SPHERES; }
    const SceneData& GetData() const { return sceneData; }

    bool NeedsUpdate() const { return needsUpdate; }
    void ResetUpdate() const { needsUpdate = false; }

private:
    SceneData sceneData{};
    mutable bool needsUpdate = true;
};
