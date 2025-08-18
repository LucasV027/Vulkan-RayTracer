#include "Serialize.h"

#include "glm/gtc/type_ptr.inl"

namespace glm {
    // ---- glm::vec3 ----
    void to_json(Json& j, const vec3& v) {
        j = Json{v.x, v.y, v.z};
    }

    void from_json(const Json& j, vec3& v) {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
    }

    // ---- glm::vec4 ----
    void to_json(Json& j, const vec4& v) {
        j = Json{v.x, v.y, v.z, v.w};
    }

    void from_json(const Json& j, vec4& v) {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }

    // ---- glm::uvec4 ----
    void to_json(Json& j, const uvec4& v) {
        j = Json{v.x, v.y, v.z, v.w};
    }

    void from_json(const Json& j, uvec4& v) {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }

    // ---- glm::mat4 ----
    void to_json(Json& j, const mat4& m) {
        float arr[16] = {0.0};

        const auto pSource = value_ptr(m);
        for (int i = 0; i < 16; ++i)
            arr[i] = pSource[i];

        j = arr;
    }

    void from_json(const Json& j, mat4& m) {
        float arr[16];
        for (int i = 0; i < 16; ++i) {
            j.at(i).get_to(arr[i]);
        }
        m = make_mat4(arr);
    }
}

// ---- CameraData ----
void to_json(Json& j, const CameraData& cameraData) {
    j = Json{
        {"cameraPosition", cameraData.cameraPosition},
        {"cameraForward", cameraData.cameraForward},
        {"cameraRight", cameraData.cameraRight},
        {"cameraUp", cameraData.cameraUp},
        {"fovRad", cameraData.fovRad},
    };
}

void from_json(const Json& j, CameraData& cameraData) {
    j.at("cameraPosition").get_to(cameraData.cameraPosition);
    j.at("cameraForward").get_to(cameraData.cameraForward);
    j.at("cameraRight").get_to(cameraData.cameraRight);
    j.at("cameraUp").get_to(cameraData.cameraUp);
    j.at("fovRad").get_to(cameraData.fovRad);
}

// ---- Material ----
void to_json(Json& j, const Material& material) {
    j = Json{
        {"color", material.color},
        {"smoothness", material.smoothness},
        {"emissionColor", material.emissionColor},
        {"emissionStrength", material.emissionStrength}
    };
}

void from_json(const Json& j, Material& material) {
    j.at("color").get_to(material.color);
    j.at("smoothness").get_to(material.smoothness);
    j.at("emissionColor").get_to(material.emissionColor);
    j.at("emissionStrength").get_to(material.emissionStrength);
}

// ---- Sphere ----
void to_json(Json& j, const Sphere& sphere) {
    j = Json{
        {"position", sphere.pos},
        {"radius", sphere.rad},
        {"material", sphere.mat}
    };
}

void from_json(const Json& j, Sphere& sphere) {
    j.at("position").get_to(sphere.pos);
    j.at("radius").get_to(sphere.rad);
    j.at("material").get_to(sphere.mat);
}

// ---- Mesh ----
void to_json(Json& j, const Mesh& mesh) {
    j = Json{
        {"start", mesh.start},
        {"count", mesh.count},
        {"minBoundingBox", mesh.minBoundingBox},
        {"maxBoundingBox", mesh.maxBoundingBox},
        {"material", mesh.mat},
        {"transform", mesh.transform},
    };
}

void from_json(const Json& j, Mesh& mesh) {
    j.at("start").get_to(mesh.start);
    j.at("count").get_to(mesh.count);
    j.at("minBoundingBox").get_to(mesh.minBoundingBox);
    j.at("maxBoundingBox").get_to(mesh.maxBoundingBox);
    j.at("material").get_to(mesh.mat);
    j.at("transform").get_to(mesh.transform);
}

// ---- SceneData ----
void to_json(Json& j, const SceneData& sceneData) {
    std::vector<Sphere> activeSpheres(sceneData.spheres, sceneData.spheres + sceneData.sphereCount);
    std::vector<glm::vec4> activeVertices(sceneData.vertices, sceneData.vertices + sceneData.verticesCount);;
    std::vector<glm::uvec4> activeFaces(sceneData.faces, sceneData.faces + sceneData.facesCount);;
    std::vector<Mesh> activeMeshes(sceneData.meshes, sceneData.meshes + sceneData.meshCount);

    j = Json{
        {"spheres", activeSpheres},
        {"vertices", activeVertices},
        {"faces", activeFaces},
        {"meshes", activeMeshes},
    };
}

void from_json(const Json& j, SceneData& sceneData) {
    std::vector<Sphere> spheres;
    std::vector<glm::vec4> vertices;
    std::vector<glm::uvec4> faces;
    std::vector<Mesh> meshes;

    j.at("spheres").get_to(spheres);
    j.at("vertices").get_to(vertices);
    j.at("faces").get_to(faces);
    j.at("meshes").get_to(meshes);

    sceneData.sphereCount = std::min<uint32_t>(static_cast<uint32_t>(spheres.size()), SceneData::MAX_SPHERES);
    sceneData.verticesCount = std::min<uint32_t>(static_cast<uint32_t>(vertices.size()), SceneData::MAX_VERTICES);
    sceneData.facesCount = std::min<uint32_t>(static_cast<uint32_t>(faces.size()), SceneData::MAX_FACES);
    sceneData.meshCount = std::min<uint32_t>(static_cast<uint32_t>(meshes.size()), SceneData::MAX_MESHES);

    std::copy_n(spheres.begin(), sceneData.sphereCount, sceneData.spheres);
    std::copy_n(vertices.begin(), sceneData.verticesCount, sceneData.vertices);
    std::copy_n(faces.begin(), sceneData.facesCount, sceneData.faces);
    std::copy_n(meshes.begin(), sceneData.meshCount, sceneData.meshes);
}

// ---- Scene ----
void to_json(Json& j, const Scene& scene) {
    j = Json{{"sceneData", scene.sceneData}};
}

void from_json(const Json& j, Scene& scene) {
    j.at("sceneData").get_to(scene.sceneData);
}

// ---- Camera ----
void to_json(Json& j, const Camera& camera) {
    j = Json{
        {"cameraData", camera.cameraData},
        {"fovDeg", camera.fovDeg},
    };
}

void from_json(const Json& j, Camera& camera) {
    j.at("cameraData").get_to(camera.cameraData);
    j.at("fovDeg").get_to(camera.fovDeg);
}

// ---- Raytracer ----
void to_json(Json& j, const Raytracer& raytracer) {
    j = Json{
        {"camera", *raytracer.camera},
        {"scene", raytracer.scene},
    };
}

void from_json(const Json& j, Raytracer& raytracer) {
    if (!raytracer.camera) {
        raytracer.camera = std::make_shared<Camera>();
    }

    j.at("camera").get_to(*raytracer.camera);
    j.at("scene").get_to(raytracer.scene);
}
