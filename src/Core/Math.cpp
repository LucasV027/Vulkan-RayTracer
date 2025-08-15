#include "Math.h"

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 Math::RandomVec3() {
    return glm::normalize(glm::vec3(glm::gaussRand(0.0f, 1.0f),
                                    glm::gaussRand(0.0f, 1.0f),
                                    glm::gaussRand(0.0f, 1.0f)));
}

glm::mat4 Math::ComputeTransform(const glm::vec3& translation,
                                 const glm::vec3& rotationAxis,
                                 const float rotationAngle,
                                 const float scaleFactor) {
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
    auto R = glm::mat4(1.0f);
    if (glm::length(rotationAxis) > 0.0001f) {
        R = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis);
    }

    const glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));
    return T * R * S;
}
