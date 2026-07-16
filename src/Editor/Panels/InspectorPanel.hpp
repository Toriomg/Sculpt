// Shows and edits the components (Name, Transform, Mesh) of the currently selected entity.
#pragma once
#include "Editor/Panels/Panel.hpp"
#include "Core/Entity.hpp"
#include "Core/Components/Component.hpp"

class Scene;
class SelectionContext;
class HistorySystem;

class InspectorPanel : public Panel {
public:
    InspectorPanel(Scene* scene, SelectionContext* selectionContext);
    void OnImGuiRender() override;

private:
    Scene*            m_Scene;
    SelectionContext*  m_SelectionContext;
    HistorySystem*     m_HistSys           = nullptr;

    Entity             m_SnapshotEntity    = entt::null;
    TransformComponent m_TransformSnapshot;

    Entity m_TexturePathEntity             = entt::null;
    char   m_TexturePathBuf[256]           = {};
};
