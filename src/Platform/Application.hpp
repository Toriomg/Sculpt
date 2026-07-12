// Top-level application: owns the Window and LayerStack, runs the main loop, and routes events down the stack.
#pragma once

#include <string>
#include <memory>
#include "Platform/System/Input/Input.hpp"
#include "Platform/System/Window/Window.hpp"
#include "Platform/Layers/LayerStack.hpp"

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
    bool m_Running = true;

	LayerStack m_LayerStack;

    bool OnWindowClose(WindowCloseEvent& e);
};