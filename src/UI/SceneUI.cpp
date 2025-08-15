#include "SceneUI.h"

 #include <format>

// ImGui
#include <imgui.h>

// GLM
#include <glm/gtc/type_ptr.hpp>

#include "BaseUI.h"

bool UI::DrawMaterial(Material& mat) {
    bool changed = false;

    ImGui::SeparatorText("Material");
    {
        ImGui::Indent();
        ImGui::SeparatorText("Object");
        {
            ImGui::Indent();
            changed |= ImGui::ColorEdit3("Color", glm::value_ptr(mat.color));
            changed |= ImGui::SliderFloat("Smoothness", &mat.smoothness, MIN_SMOOTHNESS, MAX_SMOOTHNESS);
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
            changed |= ImGui::ColorEdit3("Color##", glm::value_ptr(mat.emissionColor));
            changed |= ImGui::SliderFloat("Strength", &mat.emissionStrength, MIN_EMISSION_STRENGTH,
                                          MAX_EMISSION_STRENGTH);
            ImGui::Unindent();
        }
        ImGui::Unindent();
    }

    return changed;
}

bool UI::DrawSphere(Sphere& sphere) {
    bool changed = false;

    ImGui::SeparatorText("Geometry");
    {
        ImGui::Indent();
        changed |= ImGui::SliderFloat3("Position", glm::value_ptr(sphere.pos), MIN_POS, MAX_POS);
        changed |= ImGui::SliderFloat("Radius", &sphere.rad, MIN_SPHERE_RADIUS, MAX_SPHERE_RADIUS);
        ImGui::Unindent();
    }

    changed |= DrawMaterial(sphere.mat);

    ImGui::NewLine();

    return changed;
}

bool UI::DrawTransform(glm::mat4& transform) {
    static float scaleFactor = 1.0f;
    static auto translation = glm::vec3(0.0f);
    static float rotationAngle = 0.0f;
    static auto rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

    bool transformUpdate = false;

    ImGui::SeparatorText("Translation");
    {
        ImGui::Indent();
        if (ImGui::SliderFloat3("Offset", glm::value_ptr(translation), -100.f, 100.f, "%.2f")) {
            transformUpdate = true;
        }
        ImGui::Unindent();
    }

    ImGui::SeparatorText("Rotation");
    {
        ImGui::Indent();
        if (ImGui::DragFloat3("Axis", glm::value_ptr(rotationAxis), 0.1f, -1.0f, 1.0f, "%.2f") ||
            ImGui::DragFloat("Angle", &rotationAngle, 1.f, 0.0f, 180.0f, "%.2f")) {
            transformUpdate = true;
        }
        ImGui::Unindent();
    }

    ImGui::SeparatorText("Scale");
    {
        ImGui::Indent();
        if (ImGui::SliderFloat("Factor", &scaleFactor, 0.01f, 10.0f, "%.2f")) {
            transformUpdate = true;
        }
        ImGui::Unindent();
    }

    if (transformUpdate) {
        Mesh::ComputeTransform(transform, translation, rotationAxis, rotationAngle, scaleFactor);
    }

    return transformUpdate;
}

bool UI::DrawMesh(Mesh& mesh) {
    bool changed = false;

    ImGui::SeparatorText("Indices");
    {
        ImGui::Indent();

        ImGui::Text("Start : %u", mesh.start);
        ImGui::Text("Count : %u", mesh.count);

        ImGui::Unindent();
    }

    ImGui::SeparatorText("Transform");
    {
        ImGui::Indent();
        changed |= DrawTransform(mesh.transform);
        ImGui::Unindent();
    }

    changed |= DrawMaterial(mesh.mat);

    ImGui::NewLine();

    return changed;
}

void UI::DrawScene(Scene& scene) {
    auto& sceneData = scene.GetData();
    bool changed = false;
    static std::string filename;

    if (ImGui::TreeNode("Scene")) {
        {
            ImGui::Text("Meshes[%u]", sceneData.meshCount);
            if (!scene.MeshFull()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("+")) {
                    ImGui::OpenPopup("LoadPopup");
                }
                if (ImGui::BeginPopupModal("LoadPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    LoadPopup(filename, ".obj", [&](const std::filesystem::path& filepath) {
                        scene.AddMesh(filepath);
                        changed = true;
                    });
                    ImGui::EndPopup();
                }
            }

            ImGui::Separator();
            ImGui::Indent();

            for (uint32_t i = 0; i < sceneData.meshCount; i++) {
                if (ImGui::TreeNode(std::format("Mesh[{}]", i).c_str())) {
                    changed |= DrawMesh(sceneData.meshes[i]);
                    ImGui::TreePop();
                }
            }
            ImGui::Unindent();
        }

        ImGui::NewLine();

        {
            ImGui::Text("Spheres[%u]", sceneData.sphereCount);
            if (!scene.SphereFull()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("+##")) {
                    scene.AddSphere();
                    changed = true;
                }
            }

            ImGui::Separator();

            ImGui::Indent();

            for (uint32_t i = 0; i < sceneData.sphereCount; i++) {
                ImGui::PushID(i);
                if (ImGui::SmallButton("x")) {
                    scene.RemoveSphere(i);
                    changed = true;
                }
                ImGui::PopID();
                ImGui::SameLine();

                if (ImGui::TreeNode(std::format("Sphere[{}]", i).c_str())) {
                    changed |= DrawSphere(sceneData.spheres[i]);
                    ImGui::TreePop();
                }
            }
            ImGui::Unindent();
        }
        ImGui::TreePop();
    }

    if (changed) {
        scene.NotifyUpdate();
    }
}
