#include "BVH.h"

#include <ranges>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/vector_common.hpp>

size_t BVH_Node::Depth() const {
    if (left && right) return 1 + std::max(left->Depth(), right->Depth());
    if (left) return 1 + left->Depth();
    if (right) return 1 + right->Depth();
    return 0;
}

BVH_Node::~BVH_Node() {
    delete left;
    delete right;
}

size_t BVH_Node::Flatten(std::vector<BVH_FlattenNode>& nodes) const {
    const auto flat = BVH_FlattenNode{
        .bbox = bbox,
        .start = 0,
        .count = 0,
    };

    nodes.push_back(flat);
    const size_t idx = nodes.size() - 1;

    if (left) nodes[idx].left = left->Flatten(nodes);
    if (right) nodes[idx].right = right->Flatten(nodes);

    return idx;
}

BVH::BVH(const std::vector<Triangle>& triangles, const size_t maxDepth) {
    assert(triangles.size() >= 2);

    root = new BVH_Node({.triangles = triangles});
    ComputeNode(root, 0, maxDepth);
}

BVH::~BVH() {
    delete root;
}

BVH_Scene BVH::ToGPUData() const {
    BVH_Scene scene;
    if (!root) return scene;

    root->Flatten(scene.nodes);

    const auto& triangles = root->triangles;
    scene.triangles.reserve(triangles.size());

    std::vector<Triangle> trianglesInBbox;
    trianglesInBbox.reserve(triangles.size());

    for (auto& [bbox, left, right, start, count] : scene.nodes) {
        if (left != 0 || right != 0) continue;

        ComputeTrianglesInBoundingBox(triangles, bbox, trianglesInBbox);
        start = scene.triangles.size();
        count = trianglesInBbox.size();
        scene.triangles.insert(scene.triangles.end(), trianglesInBbox.begin(), trianglesInBbox.end());
    }

    return scene;
}

BoundingBox BVH::ComputeBoundingBox(const std::vector<Triangle>& triangles) {
    auto bbMin = glm::vec3(FLT_MAX);
    auto bbMax = glm::vec3(-FLT_MAX);

    for (const auto& triangle : triangles) {
        bbMin = glm::min(bbMin, triangle.a, triangle.b, triangle.c);
        bbMax = glm::max(bbMax, triangle.a, triangle.b, triangle.c);
    }

    return {.min = bbMin, .max = bbMax};
}

Cut BVH::ComputeCut(const std::vector<Triangle>& triangles, const BoundingBox& bbox) {
    const glm::vec3 dimensions = bbox.max - bbox.min;
    const uint8_t cutIdx = (dimensions.x >= dimensions.y && dimensions.x >= dimensions.z)
                               ? 0
                               : (dimensions.y >= dimensions.z)
                               ? 1
                               : 2;

    std::vector<Triangle> sorted = triangles;
    std::ranges::sort(sorted, [&](const Triangle& t1, const Triangle& t2) {
        return GetCenter(t1)[cutIdx] < GetCenter( t2)[cutIdx];
    });

    std::vector<Triangle> lower, higher;
    const size_t mid = sorted.size() / 2;
    lower.assign(sorted.begin(), sorted.begin() + mid);
    higher.assign(sorted.begin() + mid, sorted.end());

    return {.left = lower, .right = higher};
}

void BVH::ComputeNode(BVH_Node* node, const uint32_t depth, const uint32_t maxDepth) {
    if (!node) return;

    node->bbox = ComputeBoundingBox(node->triangles);
    if (node->triangles.size() < MIN_TRIANGLES_PER_BOX) return;
    if (depth == maxDepth) return;

    auto [left, right] = ComputeCut(node->triangles, node->bbox);
    node->left = new BVH_Node({.triangles = left});
    node->right = new BVH_Node({.triangles = right});

    ComputeNode(node->right, depth + 1, maxDepth);
    ComputeNode(node->left, depth + 1, maxDepth);
}

void BVH::ComputeTrianglesInBoundingBox(const std::vector<Triangle>& triangles,
                                        const BoundingBox& bb,
                                        std::vector<Triangle>& out) {
    out.clear();
    for (const auto& triangle : triangles) {
        if (glm::min(triangle.a, triangle.b, triangle.c, bb.min) == bb.min &&
            glm::max(triangle.a, triangle.b, triangle.c, bb.max) == bb.max) {
            out.push_back({
                .a = triangle.a,
                .b = triangle.b,
                .c = triangle.c
            });
        }
    }
}

glm::vec3 BVH::GetCenter(const Triangle& triangle) {
    return {
        (triangle.a.x + triangle.b.x + triangle.c.x) / 3.f,
        (triangle.a.y + triangle.b.y + triangle.c.y) / 3.f,
        (triangle.a.z + triangle.b.z + triangle.c.z) / 3.f
    };
}
