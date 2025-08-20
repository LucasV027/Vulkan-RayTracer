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

struct LAYOUT_STD140 Sphere {
    static constexpr size_t MAX_SPHERES = 20;

    glm::vec3 pos;
    float rad;
    Material mat;
};

struct LAYOUT_STD140 Mesh {
    uint32_t start;
    uint32_t count;
    PAD(2);
    glm::vec3 minBoundingBox;
    PAD(1);
    glm::vec3 maxBoundingBox;
    PAD(1);
    Material mat;
    glm::mat4 transform = glm::mat4(1.0f);
};

struct LAYOUT_STD140 SceneData {
    static constexpr size_t MAX_VERTICES = 1000;
    static constexpr size_t MAX_FACES = 1000;
    static constexpr size_t MAX_MESHES = 5;

    glm::vec4 vertices[MAX_VERTICES];
    glm::uvec4 faces[MAX_FACES];
    Mesh meshes[MAX_MESHES];

    uint32_t sphereCount;
    uint32_t verticesCount;
    uint32_t meshCount;
    uint32_t facesCount;
};

struct LAYOUT_STD140 Triangle {
    static constexpr size_t MAX_TRIANGLES = 2000;

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
    static constexpr size_t MAX_BVH_NODES = 500;

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

