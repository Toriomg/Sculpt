// Top-level application: owns the Window and LayerStack, runs the main loop, and routes events down
// the stack.
#pragma once

#include "Platform/Layers/LayerStack.hpp"
#include "Platform/System/Input/Input.hpp"
#include "Platform/System/Window/Window.hpp"
#include <memory>
#include <string>

class ImGuiLayer;

class Application {
public:
    Application(std::string  name = "3D MODELER", unsigned int width = 1470,
                unsigned int height = 810);
    ~Application();

    void Run();
    void OnEvent(Event& event);

private:
    std::unique_ptr<Window> m_Window;
    std::string m_AppName;
    bool m_Running = true;

    LayerStack m_LayerStack;
    ImGuiLayer* m_ImGuiLayer = nullptr;  // non-owning; LayerStack is the owner

    bool OnWindowClose(WindowCloseEvent& e);
};
