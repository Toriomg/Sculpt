#include "Editor/Panels/MainMenuBar.hpp"
#include "Renderer/Renderer.hpp"
#include "imgui.h"

MainMenuBar::MainMenuBar(std::function<void()> onQuit)
    : m_OnQuit(std::move(onQuit)) {}

void MainMenuBar::OnImGuiRender() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit"))
                m_OnQuit();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("ImGui Demo", nullptr, &ShowDemo);
            if (ImGui::MenuItem("Selection Debug", nullptr, &ShowSelectionDebug))
                Renderer::SetDebugSelectionMode(ShowSelectionDebug);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (ShowDemo)
        ImGui::ShowDemoWindow(&ShowDemo);
}
