// Top menu bar with File and Debug menus; uses a quit callback to close the application.
#pragma once
#include "Editor/Panels/Panel.hpp"
#include <functional>

class MainMenuBar : public Panel {
public:
    explicit MainMenuBar(std::function<void()> onQuit);
    void OnImGuiRender() override;

    bool ShowDemo = false;

private:
    std::function<void()> m_OnQuit;
};
