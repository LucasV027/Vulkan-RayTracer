#include "RaytracerUI.h"

#include "CameraUI.h"
#include "SceneUI.h"

void UI::DrawRaytracer(Raytracer& raytracer) {
    if (DrawCamera(raytracer.GetCamera())) {
        raytracer.SetDirty(DirtyFlags::Camera);
    }

    if (DrawScene(raytracer.GetScene())) {
        raytracer.SetDirty(DirtyFlags::Scene);
    }
}
