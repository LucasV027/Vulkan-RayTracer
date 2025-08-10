#include "Scene.h"

#include <format>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "Core/Log.h"

Scene::Scene() {
    sceneData.count = 3u;

    sceneData.spheres[0] = Sphere(glm::vec3(0.0, 0.0, -5.0), 1.0,
                                  Material(glm::vec3(0.0), 0.0, glm::vec3(1.0, 0.0, 0.0)));
    sceneData.spheres[1] = Sphere(glm::vec3(9.0, -40.0, -8.0), 30.0,
                                  Material(glm::vec3(1.0, 1.0, 0.7), 5.0, glm::vec3(0.0)));
    sceneData.spheres[2] = Sphere(glm::vec3(0.0, 52.0, -6.0), 50.0, Material(glm::vec3(1.0), 0.0, glm::vec3(0.2)));
}

void Scene::DrawUI() {
    if (ImGui::TreeNode("Scene")) {
        if (ImGui::Button("Add Sphere")) {
            if (!AddSphere()) {
                LOGE("Failed to add sphere");
            } else {
                needsUpdate = true;
            }
        }

        if (ImGui::TreeNode(std::format("Data [{}]", sceneData.count).c_str())) {
            for (uint32_t i = 0; i < sceneData.count; i++) {
                if (ImGui::TreeNode(std::format("Sphere [{}]", i).c_str())) {
                    auto& [pos, rad, mat] = sceneData.spheres[i];

                    needsUpdate |= ImGui::SliderFloat3("Position", glm::value_ptr(pos), MIN_POS, MAX_POS);
                    needsUpdate |= ImGui::SliderFloat("Radius", &rad, MIN_SPHERE_RADIUS, MAX_SPHERE_RADIUS);
                    if (ImGui::TreeNode("Material")) {
                        needsUpdate |= ImGui::ColorEdit3("Object Color", glm::value_ptr(mat.color));
                        needsUpdate |= ImGui::ColorEdit3("Emission Color", glm::value_ptr(mat.emissionColor));
                        needsUpdate |= ImGui::SliderFloat("Emission Strength", &mat.emissionStrength,
                                                          MIN_EMISSION_STRENGTH,
                                                          MAX_EMISSION_STRENGTH);
                        ImGui::TreePop();
                    }

                    if (ImGui::Button("Remove")) {
                        RemoveSphere(i);
                        needsUpdate = true;
                    }
                    ImGui::NewLine();


                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

glm::vec3 RandomColor() {
    return glm::normalize(glm::vec3(glm::gaussRand(0.0f, 1.0f), glm::gaussRand(0.0f, 1.0f),
                                    glm::gaussRand(0.0f, 1.0f)));
}

bool Scene::AddSphere() {
    if (sceneData.count == MAX_SPHERES) return false;

    sceneData.spheres[sceneData.count] = Sphere(glm::vec3(0.0f, 0.0f, -5.0f) + RandomColor() * 3.f, 1.0f,
                                                Material(glm::vec3(0.0f), 0.0f, RandomColor()));
    sceneData.count++;

    return true;
}

void Scene::RemoveSphere(const uint32_t idx) {
    if (idx > sceneData.count) return;

    sceneData.count--;
    for (uint32_t i = sceneData.count; i > idx; i--) {
        sceneData.spheres[i] = sceneData.spheres[i - 1];
    }
}
