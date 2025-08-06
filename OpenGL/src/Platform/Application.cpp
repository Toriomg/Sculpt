#include "Application.h"

Application::Application(const std::string& name, unsigned int width, unsigned int height)
    : m_Window(nullptr), m_AppName(name), m_Width(width), m_Height(height)
{
	Log::Init(); // Initialize the logging system

    m_Window = std::unique_ptr<Window>(Window::Create());
    Input::Init(m_Window.get());

	m_Window->SetVSync(true);
    m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    // Setup Time
	Time::Init();

    m_LayerStack.PushLayer(new EditorLayer());

	CORE_LOG_INFO("OpenGL Initialized");
}

Application::~Application()
{
	Input::Shutdown(); // Shutdown the input system
    CORE_LOG_INFO("Program CORRECTLY ended");
}

void Application::Run()
{
    while (m_Running)
    {
		Time::Update();
		Input::OnUpdate(); // Update the input system
        for (Layer* layer : m_LayerStack) {
            layer->OnUpdate(Time::GetDeltaTime());
        }
        m_Window->OnUpdate();
    }
}

void Application::OnEvent(Event& e) {
    // Use the dispatcher to route the event to the correct handler
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(std::bind(&Application::OnWindowClose, this, std::placeholders::_1));

    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
        // If a layer has handled the event, stop processing.
        if (e.Handled) {
            break;
        }
        (*it)->OnEvent(e);
    }
}

bool Application::OnWindowClose(WindowCloseEvent& e) {
    m_Running = false;
    CORE_LOG_INFO("Window close event received. Shutting down.");
    return true; // Return true: we handled it, no other layer needs it.
}