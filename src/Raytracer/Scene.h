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

    const SceneData& GetSceneData() const;

    const std::vector<Sphere>& GetSpheres() const { return spheres; }
    const std::vector<Mesh>& GetMeshes() const { return meshes; }
    const std::vector<Triangle>& GetTriangles() const { return triangles; }
    const std::vector<BVH_FlattenNode>& GetBVHNodes() const { return bvhNodes; }

    std::vector<Sphere>& GetSpheres() { return spheres; }

private:
    mutable SceneData sceneData = {};

    std::vector<Mesh> meshes;
    std::vector<Triangle> triangles;
    std::vector<BVH_FlattenNode> bvhNodes;
    std::vector<Sphere> spheres;
};
