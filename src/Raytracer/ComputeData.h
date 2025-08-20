#pragma once

#include <vector>

#include <glm/glm.hpp>

#define PAD(n) float UNIQUE(_pad, __LINE__)[n]
#define UNIQUE(base, line) UNIQUE_NAME(base, line)
#define UNIQUE_NAME(base, line) base##line

struct alignas(16) CameraData {
    glm::vec3 cameraPosition;
    PAD(1);

    glm::vec3 cameraForward;
    PAD(1);

    glm::vec3 cameraRight;
    PAD(1);

    glm::vec3 cameraUp;
    float fovRad;
};

struct alignas(16) SceneData {
    uint32_t numTriangles;
    uint32_t numSpheres;
    uint32_t numMeshes;
    PAD(1);
};

struct alignas(16) Material {
    glm::vec3 color;
    float smoothness;
    glm::vec3 emissionColor;
    float emissionStrength;
};

struct alignas(16) Mesh {
    uint32_t start;
    PAD(3);
    Material mat;
};

struct alignas(16) Triangle {
    glm::vec3 a;
    PAD(1);
    glm::vec3 b;
    PAD(1);
    glm::vec3 c;
    PAD(1);
};

struct alignas(16) BoundingBox {
    glm::vec3 min;
    PAD(1);
    glm::vec3 max;
    PAD(1);
};

struct alignas(16) BVH_FlattenNode {
    BoundingBox bbox;

    uint32_t left;
    uint32_t right;

    uint32_t start;
    uint32_t count;
};

struct alignas(16) Sphere {
    glm::vec3 pos;
    float rad;
    Material mat;
};


