#include "Editor/Panels/InspectorPanel.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Core/Systems/TransformCommand.hpp"
#include "Core/Components/Component.hpp"
#include "Renderer/Material.hpp"
#include "AssetManager/AssetManager.hpp"
#include "imgui.h"
#include <array>
#include <algorithm>

InspectorPanel::InspectorPanel(Scene* scene, SelectionContext* selectionContext)
    : m_Scene(scene), m_SelectionContext(selectionContext) {
    m_HistSys = scene->GetSystem<HistorySystem>();
}

void InspectorPanel::OnImGuiRender() {
    if (!IsVisible) return;
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
            ImGui::DragFloat3("Position", &pos.x, 0.1f);
            if (ImGui::IsItemActivated()) {
                m_TransformSnapshot = tc.Transform;
                m_SnapshotEntity    = entity;
            }
            if (ImGui::IsItemActive()) {
                tc.Transform.m[0][3] = pos.x;
                tc.Transform.m[1][3] = pos.y;
                tc.Transform.m[2][3] = pos.z;
            }
            if (ImGui::IsItemDeactivatedAfterEdit() && m_HistSys && entity == m_SnapshotEntity) {
                m_HistSys->Push(std::make_unique<TransformCommand>(
                    m_Scene, entity, m_TransformSnapshot, tc.Transform));
            }

            // Diagonal holds scale for pure TRS matrices (no rotation yet).
            Vec3 scale{tc.Transform.m[0][0], tc.Transform.m[1][1], tc.Transform.m[2][2]};
            ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.001f, 100.0f);
            if (ImGui::IsItemActivated()) {
                m_TransformSnapshot = tc.Transform;
                m_SnapshotEntity    = entity;
            }
            if (ImGui::IsItemActive()) {
                tc.Transform.m[0][0] = scale.x;
                tc.Transform.m[1][1] = scale.y;
                tc.Transform.m[2][2] = scale.z;
            }
            if (ImGui::IsItemDeactivatedAfterEdit() && m_HistSys && entity == m_SnapshotEntity) {
                m_HistSys->Push(std::make_unique<TransformCommand>(
                    m_Scene, entity, m_TransformSnapshot, tc.Transform));
            }

            // Uniform scale: display the average of the three axes; dragging multiplies all
            // axes by the same ratio so non-uniform scales are preserved proportionally.
            float uniformScale = (tc.Transform.m[0][0] + tc.Transform.m[1][1] + tc.Transform.m[2][2]) / 3.0f;
            ImGui::DragFloat("Uniform Scale", &uniformScale, 0.01f, 0.001f, 100.0f);
            if (ImGui::IsItemActivated()) {
                m_TransformSnapshot = tc.Transform;
                m_SnapshotEntity    = entity;
            }
            if (ImGui::IsItemActive() && uniformScale > 0.0f) {
                float snapshotAvg = (m_TransformSnapshot.m[0][0] + m_TransformSnapshot.m[1][1] + m_TransformSnapshot.m[2][2]) / 3.0f;
                if (snapshotAvg > 0.0f) {
                    float ratio = uniformScale / snapshotAvg;
                    tc.Transform.m[0][0] = m_TransformSnapshot.m[0][0] * ratio;
                    tc.Transform.m[1][1] = m_TransformSnapshot.m[1][1] * ratio;
                    tc.Transform.m[2][2] = m_TransformSnapshot.m[2][2] * ratio;
                }
            }
            if (ImGui::IsItemDeactivatedAfterEdit() && m_HistSys && entity == m_SnapshotEntity) {
                m_HistSys->Push(std::make_unique<TransformCommand>(
                    m_Scene, entity, m_TransformSnapshot, tc.Transform));
            }
        }
    }

    if (m_Scene->HasComponent<MeshComponent>(entity)) {
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& meshComp = m_Scene->GetComponent<MeshComponent>(entity);
            ImGui::Checkbox("Wireframe", &meshComp.Wireframe);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Texture");

            bool hasTexture = meshComp.MaterialAsset && meshComp.MaterialAsset->GetTextureHandle();
            ImGui::TextDisabled(hasTexture ? "Loaded" : "None");

            // Reset the path buffer when the selected entity changes.
            if (entity != m_TexturePathEntity) {
                m_TexturePathEntity = entity;
                m_TexturePathBuf[0] = '\0';
            }

            ImGui::SetNextItemWidth(-60.0f);
            ImGui::InputText("##texpath", m_TexturePathBuf, sizeof(m_TexturePathBuf));
            ImGui::SameLine();
            if (ImGui::Button("Load##tex")) {
                if (m_TexturePathBuf[0] != '\0' && meshComp.MaterialAsset) {
                    AssetHandle handle = AssetManager::Load(m_TexturePathBuf);
                    if (handle)
                        meshComp.MaterialAsset->SetTexture(handle);
                }
            }
        }
    }

    ImGui::End();
}
