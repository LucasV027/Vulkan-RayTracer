#include "Scene.h"

#include <format>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

bool Material::DrawUI() {
    bool changed = false;

    ImGui::SeparatorText("Material");
    {
        ImGui::Indent();
        ImGui::SeparatorText("Object");
        {
            ImGui::Indent();
            changed |= ImGui::ColorEdit3("Color", glm::value_ptr(color));
            changed |= ImGui::SliderFloat("Smoothness", &smoothness, MIN_SMOOTHNESS, MAX_SMOOTHNESS);
            ImGui::Unindent();
        }
        ImGui::Unindent();
    }

    ImGui::Spacing();

    {
        ImGui::Indent();
        ImGui::SeparatorText("Emission");
        {
            ImGui::Indent();
            changed |= ImGui::ColorEdit3("Color##", glm::value_ptr(emissionColor));
            changed |= ImGui::SliderFloat("Strength", &emissionStrength, MIN_EMISSION_STRENGTH, MAX_EMISSION_STRENGTH);
            ImGui::Unindent();
        }
        ImGui::Unindent();
    }

    return changed;
}

bool Sphere::DrawUI() {
    bool changed = false;

    ImGui::SeparatorText("Geometry");
    {
        ImGui::Indent();
        changed |= ImGui::SliderFloat3("Position", glm::value_ptr(pos), MIN_POS, MAX_POS);
        changed |= ImGui::SliderFloat("Radius", &rad, MIN_SPHERE_RADIUS, MAX_SPHERE_RADIUS);
        ImGui::Unindent();
    }

    changed |= mat.DrawUI();

    ImGui::NewLine();

    return changed;
}

void Scene::DrawUI() {
    if (ImGui::TreeNode("Scene")) {
        ImGui::Text("Objects[%u]", sceneData.count);
        if (!Full()) {
            ImGui::SameLine();
            if (ImGui::SmallButton("+")) {
                AddSphere();
                needsUpdate = true;
            }
        }

        ImGui::Separator();

        ImGui::Indent();

        for (uint32_t i = 0; i < sceneData.count; i++) {
            ImGui::PushID(i);
            if (ImGui::SmallButton("x")) {
                RemoveSphere(i);
                needsUpdate = true;
            }
            ImGui::PopID();
            ImGui::SameLine();

            if (ImGui::TreeNode(std::format("Sphere[{}]", i).c_str())) {
                needsUpdate |= sceneData.spheres[i].DrawUI();
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();

        ImGui::Unindent();
    }
}

glm::vec3 RandomVec3() {
    return glm::normalize(glm::vec3(glm::gaussRand(0.0f, 1.0f),
                                    glm::gaussRand(0.0f, 1.0f),
                                    glm::gaussRand(0.0f, 1.0f)));
}

Scene::Scene() {
    sceneData.count = 3u;

    sceneData.spheres[0] = {
        .pos = {0.0f, 0.0f, -5.0f},
        .rad = 1.0f,
        .mat = {
            .color = {1.0f, 0.0f, 0.0f},
            .smoothness = 0.0f,
            .emissionColor = {0.0f, 0.0f, 0.0f},
            .emissionStrength = 0.0f,
        }
    };

    sceneData.spheres[1] = {
        .pos = {9.0f, -40.0f, -8.0f},
        .rad = 30.0f,
        .mat = {
            .color = {0.0f, 0.0f, 0.0f},
            .smoothness = 0.0f,
            .emissionColor = {1.0, 1.0, 0.7},
            .emissionStrength = 5.0f,
        }
    };

    sceneData.spheres[2] = {
        .pos = {0.0f, 52.0f, -6.0f},
        .rad = 50.0f,
        .mat = {
            .color = {0.2f, 0.2f, 0.2f},
            .smoothness = 0.0f,
            .emissionColor = {0.0f, 0.0f, 0.0f},
            .emissionStrength = 0.0f,
        }
    };
}

void Scene::AddSphere() {
    if (Full()) return;

    sceneData.spheres[sceneData.count] = {
        .pos = glm::vec3(0.0f, 0.0f, -5.0f) + RandomVec3() * 3.f,
        .rad = 1.0f,
        .mat = {
            .color = RandomVec3(),
            .smoothness = 0.0f,
            .emissionColor = RandomVec3(),
            .emissionStrength = 0.0f,
        }
    };

    sceneData.count++;
}

void Scene::RemoveSphere(const uint32_t idx) {
    if (idx >= sceneData.count) return;

    for (uint32_t i = idx; i < sceneData.count - 1; i++) {
        sceneData.spheres[i] = sceneData.spheres[i + 1];
    }

    sceneData.count--;
}
