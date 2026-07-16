// Shows and edits the components (Name, Transform, Mesh) of the currently selected entity.
#pragma once
#include "Core/Components/Component.hpp"
#include "Core/Entity.hpp"
#include "Editor/Panels/Panel.hpp"
#include <array>

class Scene;
class SelectionContext;
class HistorySystem;
class GizmoRenderer;

class InspectorPanel : public Panel {
public:
    InspectorPanel(Scene* scene, SelectionContext* selectionContext, GizmoRenderer* gizmoRenderer);
    void OnImGuiRender() override;

private:
    void DrawMultiEntityView();
    void DrawGizmoSettings();
    void DrawTransformSection(Entity entity, TransformComponent& tc);
    void DrawMeshSection(Entity entity);

    Scene* m_Scene;
    SelectionContext* m_SelectionContext;
    HistorySystem* m_HistSys       = nullptr;
    GizmoRenderer* m_GizmoRenderer = nullptr;

    Entity m_SnapshotEntity = entt::null;
    TransformComponent m_TransformSnapshot;

    Entity m_TexturePathEntity             = entt::null;
    std::array<char, 256> m_TexturePathBuf = {};
};
