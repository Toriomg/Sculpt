#include "Editor/Panels/InspectorPanel.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Core/Components/Component.hpp"
#include "Core/EditMesh/EditModeSystem.hpp"
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

void InspectorPanel::ApplyMultiDelta(Vec3 TransformComponent::* field, Vec3 v, Entity active) {
    Vec3 base{};
    for (auto const& [e, snap] : m_MultiSnapshot) {
        if (e == active) {
            base = snap.*field;
            break;
        }
    }
    Vec3 const delta = v - base;
    for (auto const& [e, snap] : m_MultiSnapshot) {
        if (m_Scene->HasComponent<TransformComponent>(e)) {
            m_Scene->GetComponent<TransformComponent>(e).*field = snap.*field + delta;
        }
    }
}

void InspectorPanel::PushMultiHistory() {
    if ((m_HistSys == nullptr) || m_MultiSnapshot.empty()) { return; }
    std::vector<std::pair<Entity, TransformComponent>> after;
    after.reserve(m_MultiSnapshot.size());
    for (auto const& [e, snapTc] : m_MultiSnapshot) {
        if (m_Scene->HasComponent<TransformComponent>(e)) {
            after.emplace_back(e, m_Scene->GetComponent<TransformComponent>(e));
        }
    }
    m_HistSys->Push(
        std::make_unique<MultiTransformCommand>(m_Scene, m_MultiSnapshot, std::move(after)));
}

void InspectorPanel::DrawMultiTransformSection() {
    Entity const active = m_SelectionContext->GetActiveEntity();
    if (!m_Scene->HasComponent<TransformComponent>(active)) { return; }
    auto& activeTc = m_Scene->GetComponent<TransformComponent>(active);

    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) { return; }

    auto captureSnapshot = [&] {
        m_MultiSnapshot.clear();
        for (entt::entity e : m_SelectionContext->GetSelectedEntities()) {
            if (m_Scene->HasComponent<TransformComponent>(e)) {
                m_MultiSnapshot.emplace_back(e, m_Scene->GetComponent<TransformComponent>(e));
            }
        }
        m_SnapshotEntity = active;
    };

    auto drag3 = [&](char const* label, Vec3 TransformComponent::* field, float speed,
                     float lo = 0.f, float hi = 0.f) {
        Vec3 v = activeTc.*field;
        ImGui::DragFloat3(label, &v.x, speed, lo, hi);
        if (ImGui::IsItemActivated()) { captureSnapshot(); }
        if (ImGui::IsItemActive()) { ApplyMultiDelta(field, v, active); }
        if (ImGui::IsItemDeactivatedAfterEdit() && m_SnapshotEntity == active) {
            PushMultiHistory();
        }
    };

    drag3("Position", &TransformComponent::Translation, 0.1f);
    drag3("Rotation", &TransformComponent::EulerDegrees, 0.5f);
    drag3("Scale", &TransformComponent::Scale, 0.01f, 0.001f, 100.f);
}

void InspectorPanel::DrawMultiEntityView() {
    ImGui::Text("%zu objects selected", m_SelectionContext->GetSelectionCount());
    ImGui::Separator();
    DrawMultiTransformSection();
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

// ---- edit mode panel --------------------------------------------------------

void InspectorPanel::DrawEditModeSection() {
    auto const& em            = m_EditModeSystem->GetEditMesh();
    entt::entity const edited = em.GetEntity();

    // Header
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
    ImGui::TextUnformatted("EDIT MODE");
    ImGui::PopStyleColor();
    if (edited != entt::null && m_Scene->HasComponent<NameComponent>(edited)) {
        ImGui::TextDisabled("%s", m_Scene->GetComponent<NameComponent>(edited).Name.c_str());
    }

    ImGui::Separator();

    // Active operation status banner (Nielsen heuristic #1: system status visibility)
    bool const grabActive    = m_EditModeSystem->IsGrabActive();
    bool const insetActive   = m_EditModeSystem->IsInsetActive();
    bool const bevelActive   = m_EditModeSystem->IsBevelActive();
    bool const loopCutActive = m_EditModeSystem->IsLoopCutActive();
    bool const anyOpActive   = grabActive || insetActive || bevelActive || loopCutActive;

    if (anyOpActive) {
        ImVec4 const statusColor{1.0f, 0.85f, 0.1f, 1.0f};
        ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
        if (grabActive) {
            ImGui::TextUnformatted("GRAB  — drag to move");
        } else if (insetActive) {
            ImGui::TextUnformatted("INSET  — drag to adjust");
        } else if (bevelActive) {
            ImGui::TextUnformatted("BEVEL  — drag to adjust");
        } else if (loopCutActive) {
            ImGui::TextUnformatted("LOOP CUT  — click edge");
        }
        ImGui::PopStyleColor();

        if (grabActive || insetActive || bevelActive) {
            float const btnW =
                (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
            if (ImGui::Button("Confirm", ImVec2(btnW, 0))) {
                if (grabActive) {
                    m_EditModeSystem->ConfirmGrab();
                } else if (insetActive) {
                    m_EditModeSystem->ConfirmInset();
                } else if (bevelActive) {
                    m_EditModeSystem->ConfirmBevel();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(btnW, 0))) {
                if (grabActive) {
                    m_EditModeSystem->CancelGrab();
                } else if (insetActive) {
                    m_EditModeSystem->CancelInset();
                } else if (bevelActive) {
                    m_EditModeSystem->CancelBevel();
                }
            }
        } else if (loopCutActive) {
            if (ImGui::Button("Cancel Loop Cut", ImVec2(-1, 0))) {
                m_EditModeSystem->SetLoopCutMode(false);
            }
        }
        ImGui::Separator();
    }

    // Element mode buttons
    ImGui::TextUnformatted("Element Mode");
    ImVec4 const activeBtn{0.85f, 0.45f, 0.05f, 1.0f};
    float const modeW =
        (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;

    auto modeButton = [&](char const* label, ElementMode mode) {
        bool const active = em.mode == mode;
        if (active) { ImGui::PushStyleColor(ImGuiCol_Button, activeBtn); }
        if (ImGui::Button(label, ImVec2(modeW, 0)) && !anyOpActive) {
            m_EditModeSystem->SetElementMode(mode);
        }
        if (active) { ImGui::PopStyleColor(); }
    };
    modeButton("Vertex", ElementMode::Vertex);
    ImGui::SameLine();
    modeButton("Edge", ElementMode::Edge);
    ImGui::SameLine();
    modeButton("Face", ElementMode::Face);

    // Selection count
    size_t selCount      = 0;
    char const* selLabel = "vertices";
    switch (em.mode) {
        case ElementMode::Vertex:
            selCount = em.selectedVertices.size();
            selLabel = "vertices";
            break;
        case ElementMode::Edge:
            selCount = em.selectedEdges.size();
            selLabel = "edges";
            break;
        case ElementMode::Face:
            selCount = em.selectedFaces.size();
            selLabel = "faces";
            break;
    }
    ImGui::Text("Selected: %zu %s", selCount, selLabel);

    ImGui::Separator();

    // Operation buttons
    ImGui::TextUnformatted("Operations");
    if (anyOpActive) { ImGui::BeginDisabled(); }

    float const opW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    if (ImGui::Button("Extrude  E", ImVec2(opW, 0))) { m_EditModeSystem->Extrude(); }
    ImGui::SameLine();
    if (ImGui::Button("Inset  I", ImVec2(opW, 0))) { m_EditModeSystem->Inset(); }

    if (ImGui::Button("Loop Cut  ^R", ImVec2(opW, 0))) {
        m_EditModeSystem->SetLoopCutMode(!m_EditModeSystem->IsLoopCutActive());
    }
    ImGui::SameLine();
    if (ImGui::Button("Bevel  ^B", ImVec2(opW, 0))) { m_EditModeSystem->Bevel(); }

    if (anyOpActive) { ImGui::EndDisabled(); }

    ImGui::Separator();
    ImGui::Text("Vertices: %zu", em.vertices.size());
    ImGui::Text("Faces:    %u", em.FaceCount());
    ImGui::Spacing();
    ImGui::TextDisabled("Tab  exit Edit Mode");
}

// ---- main render ------------------------------------------------------------

void InspectorPanel::OnImGuiRender() {
    if (!IsVisible) { return; }
    ImGui::SetNextWindowPos(ImVec2{1210.f, 20.f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{260.f, 500.f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Inspector");

    // In edit mode the inspector shows element controls and live operation status.
    if (m_EditModeSystem != nullptr && m_EditModeSystem->IsActive()) {
        DrawEditModeSection();
        ImGui::End();
        return;
    }

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
