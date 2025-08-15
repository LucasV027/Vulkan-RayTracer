#include "RaytracerUI.h"

#include "CameraUI.h"
#include "SceneUI.h"

void UI::DrawRaytracer(Raytracer& raytracer) {
    DrawCamera(raytracer.GetCamera());
    DrawScene(raytracer.GetScene());
}
