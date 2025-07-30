#include "Application.h"


Application::Application(const std::string& name, unsigned int width, unsigned int height)
    : m_Window(nullptr), m_AppName(name), m_Width(width), m_Height(height)
{
	Log::Init(); // Initialize the logging system

    m_Window = std::unique_ptr<Window>(Window::Create());
	m_Window->SetVSync(true);
    // m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    // Setup Time
	Time::Init();

    // Setup Input
	//InputManager::Get().Init(m_Window);

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
		//InputManager::Get().Update();
		Time::Update();

		//std::cout << InputManager::Get().GetMouseX() << std::endl;
        m_Window->OnUpdate();
    }
}