#pragma once

#include <vector>
#include <iostream>

#include "ComputeData.h"

struct Cut {
    std::vector<Triangle> left, right;
};

struct BVH_Node {
    std::vector<Triangle> triangles;
    BoundingBox bbox;
    BVH_Node* left;
    BVH_Node* right;

    size_t Depth() const;
    ~BVH_Node();

    size_t Flatten(std::vector<BVH_FlattenNode>& nodes) const;
};

struct BVH_Scene {
    std::vector<BVH_FlattenNode> nodes;
    std::vector<Triangle> triangles;
};

class BVH {
public:
    explicit BVH(const std::vector<Triangle>& triangles, size_t maxDepth = 10);
    ~BVH();

    size_t GetMaxDepth() const { return root->Depth(); }

    BVH_Scene ToGPUData() const;

private:
    static constexpr uint32_t MIN_TRIANGLES_PER_BOX = 8;

    static BoundingBox ComputeBoundingBox(const std::vector<Triangle>& triangles);

    static Cut ComputeCut(const std::vector<Triangle>& triangles, const BoundingBox& bbox);
    static void ComputeNode(BVH_Node* node, uint32_t depth, uint32_t maxDepth);

    static void ComputeTrianglesInBoundingBox(const std::vector<Triangle>& triangles,
                                              const BoundingBox& bb,
                                              std::vector<Triangle>& out);

    static glm::vec3 GetCenter(const Triangle& triangle);

private:
    BVH_Node* root;
};

