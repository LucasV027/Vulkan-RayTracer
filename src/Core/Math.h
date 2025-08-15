#pragma once

#include <glm/glm.hpp>

namespace Math {
    glm::vec3 RandomVec3();
    glm::mat4 ComputeTransform(const glm::vec3& translation,
                               const glm::vec3& rotationAxis,
                               float rotationAngle,
                               float scaleFactor);
}
