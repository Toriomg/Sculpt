// Top menu bar: File, Add (entity creation), Edit (undo/redo), View (panel toggles), Debug.
#pragma once
#include "Editor/Panels/Panel.hpp"
#include <functional>
#include <string>

class HistorySystem;
class EntityFactory;

class MainMenuBar : public Panel {
public:
    MainMenuBar(std::function<void()> onQuit,
                HistorySystem* histSys,
                EntityFactory* factory,
                Panel* outliner,
                Panel* inspector,
                Panel* scenePanel,
                Panel* viewport);

    void OnImGuiRender() override;

private:
    std::function<void()> m_OnQuit;
    HistorySystem*  m_HistSys;
    EntityFactory*  m_Factory;
    Panel* m_Outliner;
    Panel* m_Inspector;
    Panel* m_ScenePanel;
    Panel* m_Viewport;
    bool        m_ShowDemo           = false;
    bool        m_ShowSelectionDebug = false;
    bool        m_ShowImportModal    = false;
    bool        m_ShowErrorModal     = false;
    char        m_ImportPathBuf[256] = {};
    std::string m_ErrorMessage;
};
