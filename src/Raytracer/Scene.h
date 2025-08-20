#pragma once

#include <filesystem>

#include "BVH.h"
#include "ComputeData.h"
#include "Extern/objload.h"
#include "Serialize/Base.h"

class Scene {
public:
    Serializable(Scene);

    Scene();
    ~Scene() = default;

    void AddSphere();
    void RemoveSphere(uint32_t idx);
    void AddMesh(const std::filesystem::path& path);
    void RemoveMesh(uint32_t idx);

    bool SphereFull() const { return spheres.size() == Sphere::MAX_SPHERES; }
    bool MeshFull() const { return sceneData.meshCount == SceneData::MAX_MESHES; }

    SceneData& GetData() { return sceneData; }
    const SceneData& GetData() const { return sceneData; }

    const BVH_Scene& GetBVH() const { return bvhScene; }
    const std::vector<Sphere>& GetSpheres() const { return spheres; }
    std::vector<Sphere>& GetSpheres() { return spheres; }

    void CreateBVH(const obj::Model& model);

private:
    std::vector<Sphere> spheres;
    SceneData sceneData = {};

    std::unique_ptr<BVH> bvh;
    std::vector<Triangle> triangles;
    BVH_Scene bvhScene;
};
