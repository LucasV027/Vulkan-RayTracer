#pragma once

#include <vector>

#include <glm/glm.hpp>

#define LAYOUT_STD140 alignas(16)
#define PAD(n) float UNIQUE(_pad, __LINE__)[n]
#define UNIQUE(base, line) UNIQUE_NAME(base, line)
#define UNIQUE_NAME(base, line) base##line

struct LAYOUT_STD140 CameraData {
    glm::vec3 cameraPosition;
    PAD(1);

    glm::vec3 cameraForward;
    PAD(1);

    glm::vec3 cameraRight;
    PAD(1);

    glm::vec3 cameraUp;
    float fovRad;
};

struct LAYOUT_STD140 Material {
    glm::vec3 color;
    float smoothness;
    glm::vec3 emissionColor;
    float emissionStrength;
};

struct LAYOUT_STD140 SceneData {
    uint32_t numTriangles;
    uint32_t numSpheres;
    uint32_t numMeshes;
    PAD(1);
};

struct LAYOUT_STD140 Mesh {
    static constexpr size_t MAX_MESHES = 10;

    uint32_t start;
    PAD(3);
    Material mat;
};

struct LAYOUT_STD140 Triangle {
    static constexpr size_t MAX_TRIANGLES = 100;

    glm::vec3 a;
    PAD(1);
    glm::vec3 b;
    PAD(1);
    glm::vec3 c;
    PAD(1);
};

struct LAYOUT_STD140 BoundingBox {
    glm::vec3 min;
    PAD(1);
    glm::vec3 max;
    PAD(1);
};

struct LAYOUT_STD140 BVH_FlattenNode {
    static constexpr size_t MAX_BVH_NODES = 100;

    BoundingBox bbox;

    uint32_t left;
    uint32_t right;

    uint32_t start;
    uint32_t count;
};

struct LAYOUT_STD140 BVH_Scene {
    std::vector<BVH_FlattenNode> nodes;
    std::vector<Triangle> triangles;
};

struct LAYOUT_STD140 Sphere {
    static constexpr size_t MAX_SPHERES = 100;

    glm::vec3 pos;
    float rad;
    Material mat;
};


