// Lists all named scene entities; clicking or shift-clicking selects them via SelectionContext.
#pragma once
#include "Editor/Panels/Panel.hpp"

class Scene;
class SelectionSystem;

class OutlinerPanel : public Panel {
public:
    OutlinerPanel(Scene* scene, SelectionSystem* selectionSystem);
    void OnImGuiRender() override;

    // Called externally (e.g. Del key) to open the delete confirmation popup next frame.
    void TriggerDeleteConfirmation();

private:
    Scene*           m_Scene;
    SelectionSystem* m_SelectionSystem;
    bool             m_ShowDeleteModal = false;
};
