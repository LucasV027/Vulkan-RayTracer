#pragma once

#include <filesystem>

#include <glm/glm.hpp>

#include "Core/Base.h"

constexpr size_t MAX_SPHERES = 20;
constexpr size_t MAX_VERTICES = 10;
constexpr size_t MAX_FACES = 10;
constexpr size_t MAX_MESHES = 5;

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

struct LAYOUT_STD140 Mesh {
    uint32_t start;
    uint32_t count;
    PAD(2);
    Material mat;

    bool DrawUI();
};

struct LAYOUT_STD140 SceneData {
    Sphere spheres[MAX_SPHERES];

    glm::vec4 vertices[MAX_VERTICES];
    glm::uvec4 faces[MAX_FACES];
    Mesh meshes[MAX_MESHES];

    uint32_t sphereCount;
    uint32_t verticesCount;
    uint32_t meshCount;
    uint32_t facesCount;
};

class Scene {
public:
    Scene();
    ~Scene() = default;

    void DrawUI();
    void LoadPopup();

    void AddSphere();
    void RemoveSphere(uint32_t idx);
    void AddMesh(const std::filesystem::path& path);

    bool SphereFull() const { return sceneData.sphereCount == MAX_SPHERES; }
    bool MeshFull() const { return sceneData.meshCount == MAX_MESHES; }

    const SceneData& GetData() const { return sceneData; }

    bool NeedsUpdate() const { return needsUpdate; }
    void ResetUpdate() const { needsUpdate = false; }

private:
    SceneData sceneData{};
    mutable bool needsUpdate = true;
};
