#include "Editor/Panels/OutlinerPanel.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Core/Components/Component.hpp"
#include "imgui.h"

OutlinerPanel::OutlinerPanel(Scene* scene, SelectionSystem* selectionSystem)
    : m_Scene(scene), m_SelectionSystem(selectionSystem) {}

void OutlinerPanel::OnImGuiRender() {
    ImGui::Begin("Outliner");

    auto& selCtx = m_SelectionSystem->GetSelectionContext();
    auto view = m_Scene->GetAllEntitiesWith<NameComponent>();

    for (auto entity : view) {
        const std::string& name = view.get<NameComponent>(entity).Name;
        bool selected = selCtx.IsEntitySelected(entity);

        if (ImGui::Selectable(name.c_str(), selected)) {
            bool additive = ImGui::GetIO().KeyShift;
            selCtx.Select(entity, additive);
        }
    }

    ImGui::End();
}
