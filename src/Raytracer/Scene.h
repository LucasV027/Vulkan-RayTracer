#pragma once

#include <filesystem>

#include "ComputeData.h"

class Scene {
public:
    Scene();
    ~Scene() = default;

    void AddSphere();
    void RemoveSphere(uint32_t idx);
    void AddMesh(const std::filesystem::path& path);
    void RemoveMesh(uint32_t idx);

    bool SphereFull() const { return sceneData.sphereCount == SceneData::MAX_SPHERES; }
    bool MeshFull() const { return sceneData.meshCount == SceneData::MAX_MESHES; }

    SceneData& GetData() { return sceneData; }
    const SceneData& GetData() const { return sceneData; }

private:
    SceneData sceneData = {};
};
