#include "ApplicationUI.h"

#include <imgui.h>

#include "Application.h"
#include "RaytracerUI.h"

void UI::DrawApplication(Application& app) {
    ImGui::Begin("[INFO]");
    ImGui::SeparatorText("[APPLICATION]");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::SeparatorText("[WINDOW]");
    ImGui::Text("Window size: (%d, %d)", app.window->GetWidth(), app.window->GetHeight());
    ImGui::SeparatorText("[RAYTRACER]");
    DrawRaytracer(*app.raytracer);
    ImGui::End();
}
