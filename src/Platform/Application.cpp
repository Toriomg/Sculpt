#include "Application.hpp"

Application::Application(const std::string& name, unsigned int width, unsigned int height)
    : m_Window(nullptr), m_AppName(name), m_Width(width), m_Height(height)
{
	Log::Init(); // Initialize the logging system
	AssetManager::Init(); // Initialize the Asset Manager

    m_Window = Window::Create();
    Input::Init(m_Window.get());
	Renderer::Init(); // Initialize the Renderer

	m_Window->SetVSync(true);
    m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

    // Setup Time
	Time::Init();

    m_LayerStack.PushLayer(std::make_unique<EditorLayer>());

    // Initialize viewport and camera projection with the actual framebuffer size
    WindowResizeEvent initResize(m_Window->GetWidth(), m_Window->GetHeight());
    OnEvent(initResize);

	CORE_LOG_INFO("Application Initialized");
	LOG_SEPARATOR();
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
        for (auto& layer : m_LayerStack) {
            layer->OnUpdate(Time::GetDeltaTime());
        }
        m_Window->OnUpdate();
    }
}

void Application::OnEvent(Event& e) {
    // Use the dispatcher to route the event to the correct handler
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });

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