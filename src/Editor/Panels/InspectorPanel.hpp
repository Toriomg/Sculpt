// Shows and edits the components (Name, Transform, Mesh) of the currently selected entity.
#pragma once
#include "Editor/Panels/Panel.hpp"

class Scene;
class SelectionContext;

class InspectorPanel : public Panel {
public:
    InspectorPanel(Scene* scene, SelectionContext* selectionContext);
    void OnImGuiRender() override;

private:
    Scene*            m_Scene;
    SelectionContext*  m_SelectionContext;
};
