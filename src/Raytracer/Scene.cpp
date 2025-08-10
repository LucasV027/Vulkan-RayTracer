#include "Scene.h"

#include <format>
#include <imgui.h>
#include <glm/gtc/type_ptr.inl>

Scene::Scene() {
    sceneData.count = 3;

    sceneData.spheres[0] = Sphere(glm::vec3(0.0, 0.0, -5.0), 1.0,
                                  Material(glm::vec3(0.0), 0.0, glm::vec3(1.0, 0.0, 0.0)));
    sceneData.spheres[1] = Sphere(glm::vec3(9.0, -40.0, -8.0), 30.0,
                                  Material(glm::vec3(1.0, 1.0, 0.7), 5.0, glm::vec3(0.0)));
    sceneData.spheres[2] = Sphere(glm::vec3(0.0, 52.0, -6.0), 50.0, Material(glm::vec3(1.0), 0.0, glm::vec3(0.2)));
}

void Scene::DrawUI() {
    if (ImGui::TreeNode("Scene")) {
        for (uint32_t i = 0; i < sceneData.count; i++) {
            const auto label = std::format("Sphere [{}]", i).c_str();
            if (ImGui::TreeNode(label)) {
                auto& [pos, rad, mat] = sceneData.spheres[i];

                needsUpdate |= ImGui::SliderFloat3("Position", glm::value_ptr(pos), MIN_POS, MAX_POS);
                needsUpdate |= ImGui::SliderFloat("Radius", &rad, MIN_SPHERE_RADIUS, MAX_SPHERE_RADIUS);
                if (ImGui::TreeNode("Material")) {
                    needsUpdate |= ImGui::ColorEdit3("Object Color", glm::value_ptr(mat.color));
                    needsUpdate |= ImGui::ColorEdit3("Emission Color", glm::value_ptr(mat.emissionColor));
                    needsUpdate |= ImGui::SliderFloat("Emission Strength", &mat.emissionStrength, MIN_EMISSION_STRENGTH,
                                                      MAX_EMISSION_STRENGTH);
                    ImGui::TreePop();
                }


                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}
