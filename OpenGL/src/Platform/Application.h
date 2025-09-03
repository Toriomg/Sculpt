#pragma once

#include "glhead.h"
#include "Platform/System/Time.h"
#include "Platform/System/Input/Input.h"
#include "Platform/System/Window/Window.h"
#include "Platform/CoreUtils/Log.h"
// Layers
#include "Platform/Layers/LayerStack.h"
#include "Platform/Layers/EditorLayer.h"
#include "Platform/Layers/ServiceLayer.h"

#include "AssetManager/AssetManager.h"

class Application
{
public:
    Application(const std::string& name = "3D MODELER",
        unsigned int width = 1470, unsigned int height = 810);
    ~Application();

    void Run();
	void OnEvent(Event& event);

private:
    std::unique_ptr<Window> m_Window;
    std::string m_AppName;
    unsigned int m_Width, m_Height;
    bool m_Running = true;

	LayerStack m_LayerStack;

    bool OnWindowClose(WindowCloseEvent& e);
};