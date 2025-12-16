#include "ui.h"
#include "framework.h"
#include "dependencies/imgui/imgui.h"

static bool menuOpen = true;

void UI::Init()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.f;
    style.FrameRounding = 4.f;
}

void UI::Render()
{
    if (GetAsyncKeyState(VK_INSERT) & 1)
        menuOpen = !menuOpen;

    if (!menuOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_Always);
    ImGui::Begin("Offline CS2 UI", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Internal UI Framework");
    ImGui::Separator();

    if (ImGui::BeginTabBar("tabs"))
    {
        if (ImGui::BeginTabItem("General"))
        {
            ImGui::Text("UI only — logic lives elsewhere");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Debug"))
        {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
