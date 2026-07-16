#include "Editor/Panels/InspectorPanel.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Core/Systems/Commands/TransformCommand.hpp"
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
            // --- Position ---
            Vec3 pos = tc.Translation;
            ImGui::DragFloat3("Position", &pos.x, 0.1f);
            if (ImGui::IsItemActivated()) {
                m_TransformSnapshot = tc;
                m_SnapshotEntity    = entity;
            }
            if (ImGui::IsItemActive())
                tc.Translation = pos;
            if (ImGui::IsItemDeactivatedAfterEdit() && m_HistSys && entity == m_SnapshotEntity)
                m_HistSys->Push(std::make_unique<TransformCommand>(m_Scene, entity, m_TransformSnapshot, tc));

            // --- Rotation (Euler degrees, ZYX extrinsic) ---
            Vec3 euler = tc.GetEulerDegrees();
            ImGui::DragFloat3("Rotation", &euler.x, 0.5f);
            if (ImGui::IsItemActivated()) {
                m_TransformSnapshot = tc;
                m_SnapshotEntity    = entity;
            }
            if (ImGui::IsItemActive())
                tc.Rotation = QuatFromEulerDegrees(euler);
            if (ImGui::IsItemDeactivatedAfterEdit() && m_HistSys && entity == m_SnapshotEntity)
                m_HistSys->Push(std::make_unique<TransformCommand>(m_Scene, entity, m_TransformSnapshot, tc));

            // --- Scale ---
            Vec3 scale = tc.Scale;
            ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.001f, 100.0f);
            if (ImGui::IsItemActivated()) {
                m_TransformSnapshot = tc;
                m_SnapshotEntity    = entity;
            }
            if (ImGui::IsItemActive())
                tc.Scale = scale;
            if (ImGui::IsItemDeactivatedAfterEdit() && m_HistSys && entity == m_SnapshotEntity)
                m_HistSys->Push(std::make_unique<TransformCommand>(m_Scene, entity, m_TransformSnapshot, tc));

            // --- Uniform Scale: drags all axes by the same ratio ---
            float uniformScale = (tc.Scale.x + tc.Scale.y + tc.Scale.z) / 3.0f;
            ImGui::DragFloat("Uniform Scale", &uniformScale, 0.01f, 0.001f, 100.0f);
            if (ImGui::IsItemActivated()) {
                m_TransformSnapshot = tc;
                m_SnapshotEntity    = entity;
            }
            if (ImGui::IsItemActive() && uniformScale > 0.0f) {
                float snapshotAvg = (m_TransformSnapshot.Scale.x + m_TransformSnapshot.Scale.y + m_TransformSnapshot.Scale.z) / 3.0f;
                if (snapshotAvg > 0.0f) {
                    float ratio = uniformScale / snapshotAvg;
                    tc.Scale.x = m_TransformSnapshot.Scale.x * ratio;
                    tc.Scale.y = m_TransformSnapshot.Scale.y * ratio;
                    tc.Scale.z = m_TransformSnapshot.Scale.z * ratio;
                }
            }
            if (ImGui::IsItemDeactivatedAfterEdit() && m_HistSys && entity == m_SnapshotEntity)
                m_HistSys->Push(std::make_unique<TransformCommand>(m_Scene, entity, m_TransformSnapshot, tc));
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
