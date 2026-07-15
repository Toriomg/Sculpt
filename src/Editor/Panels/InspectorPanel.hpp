// Shows and edits the components (Name, Transform, Mesh) of the currently selected entity.
#pragma once
#include "Editor/Panels/Panel.hpp"
#include "Core/Entity.hpp"
#include "Platform/CoreUtils/Math/matx.hpp"

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

    Entity m_SnapshotEntity    = entt::null;
    Matx4f m_TransformSnapshot = Matx4f::identity();
};
