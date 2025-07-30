#pragma once


#include "glhead.h"
#include "Platform/System/Time.h"
#include "Platform/System/Input.h"
#include "Platform/System/Window.h"
#include "Platform/CoreUtils/Log.h"

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

    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnKeyPressed(KeyPressedEvent& e);
	bool OnKeyReleased(KeyReleasedEvent& e);
};