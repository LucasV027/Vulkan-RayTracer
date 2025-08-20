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
        {"material", mesh.mat},
    };
}

void from_json(const Json& j, Mesh& mesh) {
    j.at("start").get_to(mesh.start);
    j.at("material").get_to(mesh.mat);
}

// ---- SceneData ----
void to_json(Json& j, const SceneData& sceneData) {
    j = Json{
        {"numMeshes", sceneData.numMeshes},
        {"numTriangles", sceneData.numTriangles},
        {"numSpheres", sceneData.numSpheres},
    };
}

void from_json(const Json& j, SceneData& sceneData) {
    j.at("numMeshes").get_to(sceneData.numMeshes);
    j.at("numTriangles").get_to(sceneData.numTriangles);
    j.at("numSpheres").get_to(sceneData.numSpheres);
}

// ---- Scene ----
void to_json(Json& j, const Scene& scene) {
    j = Json{
        {"sceneData", scene.GetSceneData()},
        {"spheres", scene.spheres},
        // TODO save meshes; triangles; bvhNodes;
    };
}

void from_json(const Json& j, Scene& scene) {
    j.at("sceneData").get_to(scene.sceneData);
    j.at("spheres").get_to(scene.spheres);
    // TODO load meshes; triangles; bvhNodes;
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
        {"camera", raytracer.camera},
        {"scene", raytracer.scene},
    };
}

void from_json(const Json& j, Raytracer& raytracer) {
    j.at("camera").get_to(raytracer.camera);
    j.at("scene").get_to(raytracer.scene);
}
