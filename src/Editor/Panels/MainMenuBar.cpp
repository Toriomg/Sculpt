#include "Editor/Panels/MainMenuBar.hpp"
#include "Editor/EntityFactory.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Renderer/Renderer.hpp"
#include "imgui.h"
#include <string>

MainMenuBar::MainMenuBar(std::function<void()> onQuit,
                         HistorySystem* histSys,
                         EntityFactory* factory,
                         Panel* outliner,
                         Panel* inspector,
                         Panel* scenePanel,
                         Panel* viewport)
    : m_OnQuit(std::move(onQuit))
    , m_HistSys(histSys)
    , m_Factory(factory)
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

    // ---- Add ----------------------------------------------------------------
    if (ImGui::BeginMenu("Add")) {
        if (ImGui::BeginMenu("Primitives")) {
            if (ImGui::MenuItem("Cube"))         m_Factory->SpawnPrimitive(PrimitiveType::Cube);
            if (ImGui::MenuItem("Sphere"))        m_Factory->SpawnPrimitive(PrimitiveType::Sphere);
            if (ImGui::MenuItem("Pyramid"))       m_Factory->SpawnPrimitive(PrimitiveType::Pyramid);
            if (ImGui::MenuItem("Torus"))         m_Factory->SpawnPrimitive(PrimitiveType::Torus);
            if (ImGui::MenuItem("Dodecahedron"))  m_Factory->SpawnPrimitive(PrimitiveType::Dodecahedron);
            if (ImGui::MenuItem("Icosahedron"))   m_Factory->SpawnPrimitive(PrimitiveType::Icosahedron);
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Import from file..."))
            m_ShowImportModal = true;
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

    // Import modal — opened by "Add > Import from file..."
    if (m_ShowImportModal) {
        ImGui::OpenPopup("Import Model");
        m_ShowImportModal = false;
    }
    if (ImGui::BeginPopupModal("Import Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("File path (e.g. res/models/monkey.obj)");
        ImGui::SetNextItemWidth(360.0f);
        ImGui::InputText("##importpath", m_ImportPathBuf, sizeof(m_ImportPathBuf));
        ImGui::Spacing();
        if (ImGui::Button("Load", ImVec2(120, 0))) {
            if (m_ImportPathBuf[0] != '\0')
                m_Factory->SpawnFromFile(m_ImportPathBuf);
            m_ImportPathBuf[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_ImportPathBuf[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
