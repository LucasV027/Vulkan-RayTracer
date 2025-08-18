#pragma once

#include "Raytracer/ComputeData.h"

#include <nlohmann/json.hpp>

#include "Raytracer/Raytracer.h"
#include "Raytracer/Scene.h"
using Json = nlohmann::json;

namespace glm {
    // ---- glm::vec3 ----
    void to_json(Json& j, const vec3& v);
    void from_json(const Json& j, vec3& v);

    // ---- glm::vec4 ----
    void to_json(Json& j, const vec4& v);
    void from_json(const Json& j, vec4& v);

    // ---- glm::uvec4 ----
    void to_json(Json& j, const uvec4& v);
    void from_json(const Json& j, uvec4& v);

    // ---- glm::mat4 ----
    void to_json(Json& j, const mat4& m);
    void from_json(const Json& j, mat4& m);
}

// ---- CameraData ----
void to_json(Json& j, const CameraData& cameraData);
void from_json(const Json& j, CameraData& cameraData);

// ---- Material ----
void to_json(Json& j, const Material& material);
void from_json(const Json& j, Material& material);

// ---- Sphere ----
void to_json(Json& j, const Sphere& sphere);
void from_json(const Json& j, Sphere& sphere);

// ---- Mesh ----
void to_json(Json& j, const Mesh& mesh);
void from_json(const Json& j, Mesh& mesh);

// ---- SceneData ----
void to_json(Json& j, const SceneData& sceneData);
void from_json(const Json& j, SceneData& sceneData);

// ---- Scene ----
void to_json(Json& j, const Scene& scene);
void from_json(const Json& j, Scene& scene);

// ---- Camera ----
void to_json(Json& j, const Camera& camera);
void from_json(const Json& j, Camera& camera);

// ---- Raytracer ----
void to_json(Json& j, const Raytracer& raytracer);
void from_json(const Json& j, Raytracer& raytracer);
