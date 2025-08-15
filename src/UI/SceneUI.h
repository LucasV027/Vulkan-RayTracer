#pragma once

#include "Raytracer/Scene.h"

namespace UI {
    bool DrawMaterial(Material& mat);
    bool DrawSphere(Sphere& sphere);
    bool DrawTransform(glm::mat4& transform);
    bool DrawMesh(Mesh& mesh);
    void DrawScene(Scene& scene);
}
