// Lists all named scene entities; clicking or shift-clicking selects them via SelectionContext.
#pragma once
#include "Editor/Panels/Panel.hpp"

class Scene;
class SelectionSystem;

class OutlinerPanel : public Panel {
public:
    OutlinerPanel(Scene* scene, SelectionSystem* selectionSystem);
    void OnImGuiRender() override;

private:
    Scene*           m_Scene;
    SelectionSystem* m_SelectionSystem;
};
