// Top menu bar: File, Edit (undo/redo), View (panel toggles), Debug.
#pragma once
#include "Editor/Panels/Panel.hpp"
#include <functional>

class HistorySystem;

class MainMenuBar : public Panel {
public:
    MainMenuBar(std::function<void()> onQuit,
                HistorySystem* histSys,
                Panel* outliner,
                Panel* inspector,
                Panel* scenePanel,
                Panel* viewport);

    void OnImGuiRender() override;

private:
    std::function<void()> m_OnQuit;
    HistorySystem* m_HistSys;
    Panel* m_Outliner;
    Panel* m_Inspector;
    Panel* m_ScenePanel;
    Panel* m_Viewport;
    bool m_ShowDemo           = false;
    bool m_ShowSelectionDebug = false;
};
