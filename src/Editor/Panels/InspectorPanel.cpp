#include "Editor/Panels/InspectorPanel.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Core/Components/Component.hpp"
#include "imgui.h"
#include <array>
#include <algorithm>

InspectorPanel::InspectorPanel(Scene* scene, SelectionContext* selectionContext)
    : m_Scene(scene), m_SelectionContext(selectionContext) {}

void InspectorPanel::OnImGuiRender() {
    ImGui::SetNextWindowPos(ImVec2{1210.f, 20.f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{260.f, 500.f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Inspector");

    if (m_SelectionContext->GetSelectionCount() == 0) {
        ImGui::TextDisabled("Nothing selected");
        ImGui::End();
        return;
    }
    if (m_SelectionContext->GetSelectionCount() > 1) {
        ImGui::TextDisabled("Multiple objects selected");
        ImGui::End();
        return;
    }

    Entity entity = *m_SelectionContext->GetSelectedEntities().begin();

    if (m_Scene->HasComponent<NameComponent>(entity)) {
        auto& nc = m_Scene->GetComponent<NameComponent>(entity);
        std::array<char, 128> buf{};
        std::copy_n(nc.Name.begin(), std::min(nc.Name.size(), buf.size() - 1), buf.begin());
        if (ImGui::InputText("Name", buf.data(), buf.size()))
            nc.Name = buf.data();
    }

    ImGui::Separator();

    if (m_Scene->HasComponent<TransformComponent>(entity)) {
        auto& tc = m_Scene->GetComponent<TransformComponent>(entity);

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            Vec3 pos = tc.GetTranslation();
            if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                tc.Transform.m[0][3] = pos.x;
                tc.Transform.m[1][3] = pos.y;
                tc.Transform.m[2][3] = pos.z;
            }
            // Diagonal holds scale for pure TRS matrices (no rotation yet).
            Vec3 scale{tc.Transform.m[0][0], tc.Transform.m[1][1], tc.Transform.m[2][2]};
            if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.001f, 100.0f)) {
                tc.Transform.m[0][0] = scale.x;
                tc.Transform.m[1][1] = scale.y;
                tc.Transform.m[2][2] = scale.z;
            }
        }
    }

    if (m_Scene->HasComponent<MeshComponent>(entity)) {
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& meshComp = m_Scene->GetComponent<MeshComponent>(entity);
            ImGui::Checkbox("Wireframe", &meshComp.Wireframe);
        }
    }

    ImGui::End();
}
