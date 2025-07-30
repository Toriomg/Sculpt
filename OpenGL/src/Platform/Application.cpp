#include "Application.h"


Application::Application(const std::string& name, unsigned int width, unsigned int height)
    : m_Window(nullptr), m_AppName(name), m_Width(width), m_Height(height)
{
	Log::Init(); // Initialize the logging system

    m_Window = std::unique_ptr<Window>(Window::Create());
	m_Window->SetVSync(true);
    m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    // Setup Time
	Time::Init();


	CORE_LOG_INFO("OpenGL Initialized");
}

Application::~Application()
{
    CORE_LOG_INFO("Program CORRECTLY ended");
}

void Application::Run()
{
    while (m_Running)
    {
		Time::Update();

		//std::cout << InputManager::Get().GetMouseX() << std::endl;
        m_Window->OnUpdate();
    }
}

void Application::OnEvent(Event& e) {
    // Use the dispatcher to route the event to the correct handler
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseButtonPressedEvent>(std::bind(&Application::OnMouseButtonPressed, this, std::placeholders::_1));
    dispatcher.Dispatch<MouseButtonReleasedEvent>(std::bind(&Application::OnMouseButtonReleased, this, std::placeholders::_1));
    dispatcher.Dispatch<WindowCloseEvent>(std::bind(&Application::OnWindowClose, this, std::placeholders::_1));
}

bool Application::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
    LOG_INFO("Mouse button {0} was pressed!", e.GetMouseButton());
    // Return true if you want to "consume" the event
    return false;
}

bool Application::OnMouseButtonReleased(MouseButtonReleasedEvent& e) {
    LOG_INFO("Mouse button {0} was released!", e.GetMouseButton());
    // Return true if you want to "consume" the event
    return false;
}

bool Application::OnWindowClose(WindowCloseEvent& e) {
    m_Running = false;
    LOG_INFO("Window close event received. Shutting down.");
    return true; // Return true: we handled it, no other layer needs it.
}