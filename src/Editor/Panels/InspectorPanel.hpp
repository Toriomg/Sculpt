// Shows and edits the components (Name, Transform, Mesh) of the currently selected entity.
#pragma once
#include "Core/Components/Component.hpp"
#include "Core/Entity.hpp"
#include "Editor/Panels/Panel.hpp"
#include <array>
#include <utility>
#include <vector>

class Scene;
class SelectionContext;
class HistorySystem;
class GizmoRenderer;

class EditModeSystem;

class InspectorPanel : public Panel {
public:
    InspectorPanel(Scene* scene, SelectionContext* selectionContext, GizmoRenderer* gizmoRenderer);
    void OnImGuiRender() override;

    void SetEditModeSystem(EditModeSystem* sys) { m_EditModeSystem = sys; }

private:
    void DrawMultiEntityView();
    void DrawMultiTransformSection();
    void ApplyMultiDelta(Vec3 TransformComponent::* field, Vec3 v, Entity active);
    void PushMultiHistory();
    void DrawGizmoSettings();
    void DrawTransformSection(Entity entity, TransformComponent& tc);
    void DrawMeshSection(Entity entity);

    Scene* m_Scene;
    SelectionContext* m_SelectionContext;
    HistorySystem* m_HistSys         = nullptr;
    GizmoRenderer* m_GizmoRenderer   = nullptr;
    EditModeSystem* m_EditModeSystem = nullptr;

    Entity m_SnapshotEntity = entt::null;
    TransformComponent m_TransformSnapshot;
    std::vector<std::pair<Entity, TransformComponent>> m_MultiSnapshot;

    Entity m_TexturePathEntity             = entt::null;
    std::array<char, 256> m_TexturePathBuf = {};
};
