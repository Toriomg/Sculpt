#include "Application.h"


Application::Application(const std::string& name, unsigned int width, unsigned int height)
    : m_Window(nullptr), m_AppName(name), m_Width(width), m_Height(height)
{
	Log::Init(); // Initialize the logging system

    // Initialize the library
    if (!glfwInit()) {
        CORE_LOG_CRITICAL("ERROR: glfwInit() failed.");
        return;
    }

	// Set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    m_Window = glfwCreateWindow(m_Width, m_Height, m_AppName.c_str(), NULL, NULL);
    if (!m_Window)
    {
        glfwTerminate();
        CORE_LOG_CRITICAL("ERROR: glfwCreateWindow() failed.");
        return;
    }

    // Make the window's context current
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1); // Enable vsync


    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        CORE_LOG_CRITICAL("ERROR: glewInit() failed.");
        return;
    }


    // Setup OpenGL State 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // Setup Time
	Time::Init();

    // Setup Input
	InputManager::Get().Init(m_Window);

	CORE_LOG_INFO("OpenGL Initialized");
}

Application::~Application()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    CORE_LOG_INFO("Program CORRECTLY ended");
}

void Application::Run()
{
    while (!glfwWindowShouldClose(m_Window))
    {
		InputManager::Get().Update();
		Time::Update();

		//std::cout << InputManager::Get().GetMouseX() << std::endl;
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}