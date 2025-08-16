#include "Scene.h"

#include "Extern/objload.h"

#include "Core/Log.h"
#include "Core/Math.h"

Scene::Scene() {
    sceneData.sphereCount = 3u;
    sceneData.verticesCount = 0u;
    sceneData.meshCount = 0u;
    sceneData.facesCount = 0u;

    sceneData.spheres[0] = {
        .pos = {0.0f, 0.0f, -5.0f},
        .rad = 1.0f,
        .mat = {
            .color = {1.0f, 0.0f, 0.0f},
            .smoothness = 0.0f,
            .emissionColor = {0.0f, 0.0f, 0.0f},
            .emissionStrength = 0.0f,
        }
    };

    sceneData.spheres[1] = {
        .pos = {9.0f, -40.0f, -8.0f},
        .rad = 30.0f,
        .mat = {
            .color = {0.0f, 0.0f, 0.0f},
            .smoothness = 0.0f,
            .emissionColor = {1.0f, 1.0f, 0.7f},
            .emissionStrength = 5.0f,
        }
    };

    sceneData.spheres[2] = {
        .pos = {0.0f, 52.0f, -6.0f},
        .rad = 50.0f,
        .mat = {
            .color = {0.2f, 0.2f, 0.2f},
            .smoothness = 0.0f,
            .emissionColor = {0.0f, 0.0f, 0.0f},
            .emissionStrength = 0.0f,
        }
    };
}

void Scene::AddSphere() {
    if (SphereFull()) return;

    sceneData.spheres[sceneData.sphereCount] = {
        .pos = glm::vec3(0.0f, 0.0f, -5.0f) + Math::RandomVec3() * 3.f,
        .rad = 1.0f,
        .mat = {
            .color = Math::RandomVec3(),
            .smoothness = 0.0f,
            .emissionColor = Math::RandomVec3(),
            .emissionStrength = 0.0f,
        }
    };

    sceneData.sphereCount++;
}

void Scene::RemoveSphere(const uint32_t idx) {
    if (idx >= sceneData.sphereCount) return;

    for (uint32_t i = idx; i < sceneData.sphereCount - 1; i++) {
        sceneData.spheres[i] = sceneData.spheres[i + 1];
    }

    sceneData.sphereCount--;
}

void Scene::AddMesh(const std::filesystem::path& path) {
    if (MeshFull()) return;

    obj::Model model;
    try {
        model = std::move(obj::loadModelFromFile(path.string()));
    } catch (const std::exception&) {
        LOGE("Failed to load model from {}", path.string());
        return;
    }

    const auto& vertices = model.vertex;
    const auto& indices = model.faces.at("default");
    const auto verticesCount = vertices.size() / 3;
    const auto indicesCount = indices.size() / 3;

    if (sceneData.verticesCount + verticesCount > SceneData::MAX_VERTICES) {
        LOGE("Too many vertices (current {}, adding {}) ", sceneData.verticesCount, verticesCount);
        return;
    }

    if (sceneData.facesCount + indicesCount > SceneData::MAX_FACES) {
        LOGE("Too many indices (current {}, adding {}) ", sceneData.facesCount, indicesCount);
        return;
    }

    auto minBoundingBox = glm::vec3(FLT_MAX);
    auto maxBoundingBox = glm::vec3(-FLT_MAX);
    for (size_t i = sceneData.verticesCount; i < sceneData.verticesCount + verticesCount; i++) {
        const auto vertexRaw = &vertices.at((i - sceneData.verticesCount) * 3);
        const auto vertex = glm::vec3(vertexRaw[0], vertexRaw[1], vertexRaw[2]);
        minBoundingBox = glm::min(minBoundingBox, vertex);
        maxBoundingBox = glm::max(maxBoundingBox, vertex);

        sceneData.vertices[i] = glm::vec4(vertex, 1.0f);
    }

    const auto verticesStart = sceneData.verticesCount;
    for (size_t i = sceneData.facesCount; i < sceneData.facesCount + indicesCount; i++) {
        const auto face = &indices.at((i - sceneData.facesCount) * 3);
        // We offset the indices by the current start of our vertex buffer
        sceneData.faces[i] = glm::uvec4(face[0], face[1], face[2], 0) + glm::uvec4(verticesStart);
    }

    auto& newMesh = sceneData.meshes[sceneData.meshCount++];
    newMesh.count = indicesCount;
    newMesh.start = sceneData.facesCount;
    newMesh.minBoundingBox = minBoundingBox;
    newMesh.maxBoundingBox = maxBoundingBox;
    newMesh.mat = {
        .color = {0.8f, 0.5f, 0.0f},
        .smoothness = 0.0f,
        .emissionColor = {0.0f, 0.0f, 0.0f},
        .emissionStrength = 0.0f,
    };

    sceneData.verticesCount += verticesCount;
    sceneData.facesCount += indicesCount;
}

void Scene::RemoveMesh(const uint32_t idx) {
    if (idx >= sceneData.meshCount) return;

    const auto& toRemove = sceneData.meshes[idx];
    const uint32_t indicesStart = toRemove.start;
    const uint32_t indicesCount = toRemove.count;

    // Find the min and max index of the mesh triangles
    uint32_t minVertex = UINT32_MAX;
    uint32_t maxVertex = 0;

    for (uint32_t i = indicesStart; i < indicesStart + indicesCount; i++) {
        const auto& f = sceneData.faces[i];
        minVertex = std::min({minVertex, f.x, f.y, f.z});
        maxVertex = std::max({maxVertex, f.x, f.y, f.z});
    }

    const uint32_t verticesToRemove = maxVertex - minVertex + 1;

    // Remove vertices
    for (uint32_t i = minVertex; i + verticesToRemove < sceneData.verticesCount; i++) {
        sceneData.vertices[i] = sceneData.vertices[i + verticesToRemove];
    }
    sceneData.verticesCount -= verticesToRemove;

    // Update the remaining faces
    for (uint32_t i = 0; i < sceneData.facesCount; i++) {
        auto& f = sceneData.faces[i];
        if (f.x > maxVertex) f.x -= verticesToRemove;
        if (f.y > maxVertex) f.y -= verticesToRemove;
        if (f.z > maxVertex) f.z -= verticesToRemove;
    }

    // Remove faces
    for (uint32_t i = indicesStart; i + indicesCount < sceneData.facesCount; i++) {
        sceneData.faces[i] = sceneData.faces[i + indicesCount];
    }
    sceneData.facesCount -= indicesCount;

    // Remove the mesh
    for (uint32_t i = idx; i + 1 < sceneData.meshCount; i++) {
        sceneData.meshes[i] = sceneData.meshes[i + 1];
        sceneData.meshes[i].start -= indicesCount;
    }
    sceneData.meshCount--;
}

