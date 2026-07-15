#include "Editor/Panels/MainMenuBar.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Renderer/Renderer.hpp"
#include "imgui.h"
#include <string>

MainMenuBar::MainMenuBar(std::function<void()> onQuit,
                         HistorySystem* histSys,
                         Panel* outliner,
                         Panel* inspector,
                         Panel* scenePanel,
                         Panel* viewport)
    : m_OnQuit(std::move(onQuit))
    , m_HistSys(histSys)
    , m_Outliner(outliner)
    , m_Inspector(inspector)
    , m_ScenePanel(scenePanel)
    , m_Viewport(viewport) {}

void MainMenuBar::OnImGuiRender() {
    if (!ImGui::BeginMainMenuBar()) return;

    // ---- File ---------------------------------------------------------------
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit", "Alt+F4"))
            m_OnQuit();
        ImGui::EndMenu();
    }

    // ---- Edit ---------------------------------------------------------------
    if (ImGui::BeginMenu("Edit")) {
        bool canUndo = m_HistSys && m_HistSys->CanUndo();
        bool canRedo = m_HistSys && m_HistSys->CanRedo();

        std::string undoLabel = "Undo";
        std::string redoLabel = "Redo";
        if (canUndo) undoLabel += "  " + m_HistSys->GetUndoDescription();
        if (canRedo) redoLabel += "  " + m_HistSys->GetRedoDescription();

        if (ImGui::MenuItem(undoLabel.c_str(), "Ctrl+Z", false, canUndo))
            m_HistSys->Undo();
        if (ImGui::MenuItem(redoLabel.c_str(), "Ctrl+Y", false, canRedo))
            m_HistSys->Redo();

        ImGui::EndMenu();
    }

    // ---- View ---------------------------------------------------------------
    if (ImGui::BeginMenu("View")) {
        if (m_Viewport)
            ImGui::MenuItem("Viewport",   nullptr, &m_Viewport->IsVisible);
        if (m_Outliner)
            ImGui::MenuItem("Outliner",   nullptr, &m_Outliner->IsVisible);
        if (m_Inspector)
            ImGui::MenuItem("Inspector",  nullptr, &m_Inspector->IsVisible);
        if (m_ScenePanel)
            ImGui::MenuItem("Scene",      nullptr, &m_ScenePanel->IsVisible);
        ImGui::EndMenu();
    }

    // ---- Debug --------------------------------------------------------------
    if (ImGui::BeginMenu("Debug")) {
        ImGui::MenuItem("ImGui Demo", nullptr, &m_ShowDemo);
        if (ImGui::MenuItem("Selection Picking", nullptr, &m_ShowSelectionDebug))
            Renderer::SetDebugSelectionMode(m_ShowSelectionDebug);
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();

    if (m_ShowDemo)
        ImGui::ShowDemoWindow(&m_ShowDemo);
}
