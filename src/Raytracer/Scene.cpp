#include "Scene.h"

#include <format>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "Core/Log.h"
#include "Extern/objload.h"

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

bool Mesh::DrawUI() {
    bool changed = false;

    ImGui::SeparatorText("Indices");
    {
        ImGui::Indent();
        ImGui::Text("Start : %u", start);
        ImGui::Text("Count : %u", count);
        ImGui::Unindent();
    }

    changed |= mat.DrawUI();

    ImGui::NewLine();

    return changed;
}

void Scene::LoadPopup() {
    static std::string filename;

    ImGui::Text("Enter filename:");
    ImGui::InputText("##Filename", &filename, ImGuiInputTextFlags_CallbackCharFilter,
                     [](ImGuiInputTextCallbackData* data) {
                         if (isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == '-') return 0;
                         return 1;
                     });


    ImGui::Separator();

    if (ImGui::Button("Load")) {
        if (!filename.empty()) {
            auto filePath = ASSETS_PATH / static_cast<std::filesystem::path>(filename);
            filePath.replace_extension(".obj");
            AddMesh(filePath);
            filename.clear();
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
        filename.clear();
        ImGui::CloseCurrentPopup();
    }
}

void Scene::DrawUI() {
    if (ImGui::TreeNode("Scene")) {
        {
            ImGui::Text("Meshes[%u]", sceneData.meshCount);
            if (!MeshFull()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("+")) {
                    ImGui::OpenPopup("LoadPopup");
                }
                if (ImGui::BeginPopupModal("LoadPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    LoadPopup();
                    ImGui::EndPopup();
                }
            }

            ImGui::Separator();
            ImGui::Indent();

            for (uint32_t i = 0; i < sceneData.meshCount; i++) {
                if (ImGui::TreeNode(std::format("Mesh[{}]", i).c_str())) {
                    needsUpdate |= sceneData.meshes[i].DrawUI();
                    ImGui::TreePop();
                }
            }
            ImGui::Unindent();
        }

        ImGui::NewLine();

        {
            ImGui::Text("Spheres[%u]", sceneData.sphereCount);
            if (!SphereFull()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("+##")) {
                    AddSphere();
                    needsUpdate = true;
                }
            }

            ImGui::Separator();

            ImGui::Indent();

            for (uint32_t i = 0; i < sceneData.sphereCount; i++) {
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
            ImGui::Unindent();
        }
        ImGui::TreePop();
    }
}

glm::vec3 RandomVec3() {
    return glm::normalize(glm::vec3(glm::gaussRand(0.0f, 1.0f),
                                    glm::gaussRand(0.0f, 1.0f),
                                    glm::gaussRand(0.0f, 1.0f)));
}

Scene::Scene() {
    sceneData.sphereCount = 3u;
    sceneData.verticesCount = 0u;
    sceneData.meshCount = 0u;
    sceneData.facesCount = 0u;

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
            .emissionColor = {1.0f, 1.0f, 0.7f},
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
    if (SphereFull()) return;

    sceneData.spheres[sceneData.sphereCount] = {
        .pos = glm::vec3(0.0f, 0.0f, -5.0f) + RandomVec3() * 3.f,
        .rad = 1.0f,
        .mat = {
            .color = RandomVec3(),
            .smoothness = 0.0f,
            .emissionColor = RandomVec3(),
            .emissionStrength = 0.0f,
        }
    };

    sceneData.sphereCount++;
}

void Scene::RemoveSphere(const uint32_t idx) {
    if (idx >= sceneData.sphereCount) return;

    for (uint32_t i = idx; i < sceneData.sphereCount - 1; i++) {
        sceneData.spheres[i] = sceneData.spheres[i + 1];
    }

    sceneData.sphereCount--;
}

void Scene::AddMesh(const std::filesystem::path& path) {
    if (MeshFull()) return;

    obj::Model model;
    try {
        model = std::move(obj::loadModelFromFile(path.string()));
    } catch (const std::exception& e) {
        LOGE("Failed to load model from {}", path.string());
        return;
    }

    const auto& vertices = model.vertex;
    const auto& indices = model.faces.at("default");
    const auto verticesCount = vertices.size() / 3;
    const auto indicesCount = indices.size() / 3;

    if (sceneData.verticesCount + verticesCount > MAX_VERTICES) {
        LOGE("Too many vertices (current {}, adding {}) ", sceneData.verticesCount, verticesCount);
        return;
    }

    if (sceneData.facesCount + indicesCount > MAX_FACES) {
        LOGE("Too many indices (current {}, adding {}) ", sceneData.facesCount, indicesCount);
        return;
    }

    for (size_t i = sceneData.verticesCount; i < sceneData.verticesCount + verticesCount; i++) {
        const auto vertex = &vertices.at((i - sceneData.verticesCount) * 3);
        sceneData.vertices[i] = glm::vec4(vertex[0], vertex[1], vertex[2], 1.0f);
    }

    for (size_t i = sceneData.facesCount; i < sceneData.facesCount + indicesCount; i++) {
        const auto face = &indices.at((i - sceneData.facesCount) * 3);
        sceneData.faces[i] = glm::uvec4(face[0], face[1], face[2], 0);
    }

    auto& newMesh = sceneData.meshes[sceneData.meshCount++];
    newMesh.count = indicesCount;
    newMesh.start = sceneData.facesCount;
    newMesh.mat = {
        .color = {0.8f, 0.5f, 0.0f},
        .smoothness = 0.0f,
        .emissionColor = {0.0f, 0.0f, 0.0f},
        .emissionStrength = 0.0f,
    };

    sceneData.facesCount += indicesCount;
    sceneData.verticesCount += verticesCount;
    needsUpdate = true;
}

