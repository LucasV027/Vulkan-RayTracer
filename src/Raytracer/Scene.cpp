#include "Scene.h"

#include "Extern/objload.h"

#include "Core/Log.h"
#include "Core/Math.h"

Scene::Scene() {
    spheres.reserve(Sphere::MAX_SPHERES);

    spheres.emplace_back(Sphere{
        .pos = {0.0f, 0.0f, -5.0f},
        .rad = 1.0f,
        .mat = {
            .color = {1.0f, 0.0f, 0.0f},
            .smoothness = 0.0f,
            .emissionColor = {0.0f, 0.0f, 0.0f},
            .emissionStrength = 0.0f,
        }
    });

    spheres.emplace_back(Sphere{
        .pos = {9.0f, -40.0f, -8.0f},
        .rad = 30.0f,
        .mat = {
            .color = {0.0f, 0.0f, 0.0f},
            .smoothness = 0.0f,
            .emissionColor = {1.0f, 1.0f, 0.7f},
            .emissionStrength = 5.0f,
        }
    });

    spheres.emplace_back(Sphere{
        .pos = {0.0f, 52.0f, -6.0f},
        .rad = 50.0f,
        .mat = {
            .color = {0.2f, 0.2f, 0.2f},
            .smoothness = 0.0f,
            .emissionColor = {0.0f, 0.0f, 0.0f},
            .emissionStrength = 0.0f,
        }
    });
}

void Scene::AddSphere() {
    if (SphereFull()) return;

    spheres.emplace_back(Sphere{
        .pos = glm::vec3(0.0f, 0.0f, -5.0f) + Math::RandomVec3() * 3.f,
        .rad = 1.0f,
        .mat = {
            .color = Math::RandomVec3(),
            .smoothness = 0.0f,
            .emissionColor = Math::RandomVec3(),
            .emissionStrength = 0.0f,
        }
    });
}

void Scene::RemoveSphere(const uint32_t idx) {
    if (idx >= spheres.size()) return;
    spheres.erase(spheres.begin() + idx);
}

const SceneData& Scene::GetSceneData() const {
    sceneData.numMeshes = meshes.size();
    sceneData.numSpheres = spheres.size();
    sceneData.numTriangles = triangles.size();
    return sceneData;
}
