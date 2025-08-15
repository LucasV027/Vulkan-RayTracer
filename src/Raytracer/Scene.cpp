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

    for (size_t i = sceneData.verticesCount; i < sceneData.verticesCount + verticesCount; i++) {
        const auto vertex = &vertices.at((i - sceneData.verticesCount) * 3);
        sceneData.vertices[i] = glm::vec4(vertex[0], vertex[1], vertex[2], 1.0f);
    }

    for (size_t i = sceneData.facesCount; i < sceneData.facesCount + indicesCount; i++) {
        const auto face = &indices.at((i - sceneData.facesCount) * 3);
        sceneData.faces[i] = glm::uvec4(face[0], face[1], face[2], 0);
    }

    auto& newMesh = sceneData.meshes[sceneData.meshCount++];
    newMesh.count = indicesCount;
    newMesh.start = sceneData.facesCount;
    newMesh.mat = {
        .color = {0.8f, 0.5f, 0.0f},
        .smoothness = 0.0f,
        .emissionColor = {0.0f, 0.0f, 0.0f},
        .emissionStrength = 0.0f,
    };

    sceneData.facesCount += indicesCount;
    sceneData.verticesCount += verticesCount;
    needsUpdate = true;
}

