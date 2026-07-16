#include "Editor/Panels/InspectorPanel.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/Commands/MultiTransformCommand.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Editor/Gizmos/Gizmo.hpp"
#include "Editor/Gizmos/GizmoRenderer.hpp"
#include "Renderer/Material.hpp"
#include "imgui.h"
#include <algorithm>
#include <array>

InspectorPanel::InspectorPanel(Scene* scene, SelectionContext* selectionContext,
                               GizmoRenderer* gizmoRenderer)
    : m_Scene(scene), m_SelectionContext(selectionContext),
      m_HistSys(scene->GetSystem<HistorySystem>()), m_GizmoRenderer(gizmoRenderer) { }

// ---- helpers ----------------------------------------------------------------

void InspectorPanel::DrawGizmoSettings() {
    if (m_GizmoRenderer == nullptr) { return; }

    if (ImGui::CollapsingHeader("Pivot Point", ImGuiTreeNodeFlags_DefaultOpen)) {
        int pivotIdx                    = static_cast<int>(m_GizmoRenderer->GetPivotMode());
        constexpr std::array pivotNames = {"Individual Origins", "Median Point", "Active Element"};
        if (ImGui::Combo("##pivot", &pivotIdx, pivotNames.data(),
                         static_cast<int>(pivotNames.size())))
        {
            m_GizmoRenderer->SetPivotMode(static_cast<PivotMode>(pivotIdx));
        }
    }

    if (ImGui::CollapsingHeader("Snap Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto snap    = m_GizmoRenderer->GetSnapConfig();
        bool changed = false;
        changed |= ImGui::DragFloat("Translate", &snap.translate, 0.05f, 0.01f, 100.f, "%.2f");
        changed |= ImGui::DragFloat("Rotate (°)", &snap.rotate, 1.f, 1.f, 180.f, "%.0f");
        changed |= ImGui::DragFloat("Scale", &snap.scale, 0.01f, 0.01f, 10.f, "%.2f");
        if (changed) { m_GizmoRenderer->SetSnapConfig(snap); }
    }
}

void InspectorPanel::DrawMultiEntityView() {
    ImGui::Text("%zu objects selected", m_SelectionContext->GetSelectionCount());
    ImGui::Separator();
    DrawGizmoSettings();
}

void InspectorPanel::DrawTransformSection(Entity entity, TransformComponent& tc) {
    // Generic drag-with-history for any Vec3 field on the transform.
    auto drag3 = [&](char const* label, Vec3& field, float speed, float lo = 0.f, float hi = 0.f) {
        Vec3 v = field;
        ImGui::DragFloat3(label, &v.x, speed, lo, hi);
        if (ImGui::IsItemActivated()) {
            m_TransformSnapshot = tc;
            m_SnapshotEntity    = entity;
        }
        if (ImGui::IsItemActive()) { field = v; }
        if (ImGui::IsItemDeactivatedAfterEdit() &&
            (m_HistSys != nullptr) &&
            entity == m_SnapshotEntity)
        {
            m_HistSys->Push(
                std::make_unique<MultiTransformCommand>(m_Scene, entity, m_TransformSnapshot, tc));
        }
    };

    drag3("Position", tc.Translation, 0.1f);
    drag3("Rotation", tc.EulerDegrees, 0.5f);
    drag3("Scale", tc.Scale, 0.01f, 0.001f, 100.f);

    // Drags all axes by the same ratio relative to their values at activation.
    float uniformScale = (tc.Scale.x + tc.Scale.y + tc.Scale.z) / 3.0f;
    ImGui::DragFloat("Uniform Scale", &uniformScale, 0.01f, 0.001f, 100.0f);
    if (ImGui::IsItemActivated()) {
        m_TransformSnapshot = tc;
        m_SnapshotEntity    = entity;
    }
    if (ImGui::IsItemActive() && uniformScale > 0.0f) {
        float const snapshotAvg = (m_TransformSnapshot.Scale.x +
                                   m_TransformSnapshot.Scale.y +
                                   m_TransformSnapshot.Scale.z) /
                                  3.0f;
        if (snapshotAvg > 0.0f) {
            float const ratio = uniformScale / snapshotAvg;
            tc.Scale.x        = m_TransformSnapshot.Scale.x * ratio;
            tc.Scale.y        = m_TransformSnapshot.Scale.y * ratio;
            tc.Scale.z        = m_TransformSnapshot.Scale.z * ratio;
        }
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && (m_HistSys != nullptr) && entity == m_SnapshotEntity)
    {
        m_HistSys->Push(
            std::make_unique<MultiTransformCommand>(m_Scene, entity, m_TransformSnapshot, tc));
    }
}

void InspectorPanel::DrawMeshSection(Entity entity) {
    auto& meshComp = m_Scene->GetComponent<MeshComponent>(entity);
    ImGui::Checkbox("Wireframe", &meshComp.Wireframe);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Texture");

    bool const hasTexture = meshComp.MaterialAsset && meshComp.MaterialAsset->GetTextureHandle();
    ImGui::TextDisabled(hasTexture ? "Loaded" : "None");

    if (entity != m_TexturePathEntity) {
        m_TexturePathEntity = entity;
        m_TexturePathBuf[0] = '\0';
    }

    ImGui::SetNextItemWidth(-60.0f);
    ImGui::InputText("##texpath", m_TexturePathBuf.data(), m_TexturePathBuf.size());
    ImGui::SameLine();
    if (ImGui::Button("Load##tex")) {
        if (m_TexturePathBuf[0] != '\0' && meshComp.MaterialAsset != nullptr) {
            AssetHandle const handle = AssetManager::Load(m_TexturePathBuf.data());
            if (handle) { meshComp.MaterialAsset->SetTexture(handle); }
        }
    }
}

// ---- main render ------------------------------------------------------------

void InspectorPanel::OnImGuiRender() {
    if (!IsVisible) { return; }
    ImGui::SetNextWindowPos(ImVec2{1210.f, 20.f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{260.f, 500.f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Inspector");

    if (m_SelectionContext->GetSelectionCount() == 0) {
        ImGui::TextDisabled("Nothing selected");
        ImGui::End();
        return;
    }

    if (m_SelectionContext->GetSelectionCount() > 1) {
        DrawMultiEntityView();
        ImGui::End();
        return;
    }

    Entity const entity = *m_SelectionContext->GetSelectedEntities().begin();

    if (m_Scene->HasComponent<NameComponent>(entity)) {
        auto& nc = m_Scene->GetComponent<NameComponent>(entity);
        std::array<char, 128> buf{};
        std::copy_n(nc.Name.begin(), std::min(nc.Name.size(), buf.size() - 1), buf.begin());
        if (ImGui::InputText("Name", buf.data(), buf.size())) { nc.Name = buf.data(); }
    }

    ImGui::Separator();

    if (m_Scene->HasComponent<TransformComponent>(entity)) {
        auto& tc = m_Scene->GetComponent<TransformComponent>(entity);
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawTransformSection(entity, tc);
        }
    }

    if (m_Scene->HasComponent<MeshComponent>(entity)) {
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawMeshSection(entity);
        }
    }

    DrawGizmoSettings();

    ImGui::End();
}
