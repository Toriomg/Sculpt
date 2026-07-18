#include "Editor/Panels/OutlinerPanel.hpp"
#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "imgui.h"
#include <vector>

OutlinerPanel::OutlinerPanel(Scene* scene, SelectionSystem* selectionSystem)
    : m_Scene(scene), m_SelectionSystem(selectionSystem) { }

void OutlinerPanel::TriggerDeleteConfirmation() {
    if (m_SelectionSystem->GetSelectionContext().GetSelectionCount() > 0) {
        m_ShowDeleteModal = true;
    }
}

void OutlinerPanel::OnImGuiRender() {
    if (!IsVisible) { return; }
    ImGui::SetNextWindowPos(ImVec2{5.f, 20.f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{290.f, 400.f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Outliner");

    if (m_InEditMode) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
        ImGui::TextUnformatted("-- EDIT MODE --");
        if (!m_EditedName.empty()) { ImGui::TextUnformatted(m_EditedName.c_str()); }
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::TextDisabled("Tab to exit Edit Mode");
        ImGui::End();
        return;
    }

    auto& selCtx = m_SelectionSystem->GetSelectionContext();
    auto view    = m_Scene->GetAllEntitiesWith<NameComponent>();

    for (auto entity : view) {
        std::string const& name = view.get<NameComponent>(entity).Name;
        bool const selected     = selCtx.IsEntitySelected(entity);

        if (ImGui::Selectable(name.c_str(), selected)) {
            bool const additive = ImGui::GetIO().KeyShift;
            selCtx.Select(entity, additive);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    bool const hasSelection = selCtx.GetSelectionCount() > 0;
    if (!hasSelection) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Delete Selected", ImVec2(-1, 0))) { m_ShowDeleteModal = true; }
    if (!hasSelection) { ImGui::EndDisabled(); }

    // Confirmation modal — must be triggered via OpenPopup once, then handled every frame.
    if (m_ShowDeleteModal) {
        ImGui::OpenPopup("Delete?");
        m_ShowDeleteModal = false;
    }
    if (ImGui::BeginPopupModal("Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        size_t const count = selCtx.GetSelectionCount();
        ImGui::Text("Permanently delete %zu object(s)?", count);
        ImGui::Spacing();
        for (auto entity : selCtx.GetSelectedEntities()) {
            if (m_Scene->HasComponent<NameComponent>(entity)) {
                ImGui::TextDisabled("  %s",
                                    m_Scene->GetComponent<NameComponent>(entity).Name.c_str());
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            // Snapshot before clearing selection since ClearSelection modifies the set.
            std::vector<Entity> const toDelete(selCtx.GetSelectedEntities().begin(),
                                               selCtx.GetSelectedEntities().end());
            selCtx.ClearSelection();
            for (Entity const e : toDelete) { m_Scene->DestroyEntity(e); }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}
