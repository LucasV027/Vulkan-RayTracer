#include "Raytracer.h"

#include <cmath>

#include <imgui.h>

Raytracer::Raytracer(const uint32_t width, const uint32_t height) : width(width), height(height) {
    uniforms = {
        .frameIndex = 0,
        .fov = 90.f * (M_PI / 180),
        .cameraPosition = {0.f, 0.f, 0.f},
        .cameraForward = {0.f, 0.f, -1.f},
        .cameraRight = {1.f, 0.f, 0.f},
        .cameraUp = {0.f, 1.f, 0.f},
    };
}

void Raytracer::Update() {
    uniforms.frameIndex++;
}

void Raytracer::RenderUI() {
    bool change = false;
    ImGui::Text("Raytracer");
    change |= ImGui::SliderFloat3("Camera position", &uniforms.cameraPosition[0], -100.f, 100.f);
    if (change) uniforms.frameIndex = 0;
}

void Raytracer::OnResize(const uint32_t newWidth, const uint32_t newHeight) {
    width = newWidth;
    height = newHeight;
    uniforms.frameIndex = 0;
}
