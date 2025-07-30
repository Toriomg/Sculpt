#include "GlfwWindow.h"
#include "Platform/CoreUtils/Log.h"



// This is where we tell the factory method which implementation to use.
Window* Window::Create(const std::string& title, unsigned int width, unsigned int height) {
    return new GlfwWindow(title, width, height);
}

GlfwWindow::GlfwWindow(const std::string& title, unsigned int width, unsigned int height) {
    Init(title, width, height);
}

GlfwWindow::~GlfwWindow() {
    Shutdown();
}

void GlfwWindow::Init(const std::string& title, unsigned int width, unsigned int height) {
    m_Data.Title = title;
    m_Data.Width = width;
    m_Data.Height = height;

    CORE_LOG_INFO("Creating window {0} ({1}, {2})", title, width, height);

    // Initialize GLFW
    if (!glfwInit()) {
        CORE_LOG_CRITICAL("ERROR: glfwInit() failed.");
        return;
    }

    // Set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow((int)width, (int)height, m_Data.Title.c_str(), nullptr, nullptr);
    if (!m_Window)
    {
        glfwTerminate();
        CORE_LOG_CRITICAL("ERROR: glfwCreateWindow() failed.");
        return;
    
    }
    glfwMakeContextCurrent(m_Window);
    if (glewInit() != GLEW_OK) {
        CORE_LOG_CRITICAL("FATAL: Failed to initialize GLEW!");
        // You should probably throw an exception or assert here
    }

    CORE_LOG_INFO("GLEW Initialized");
    CORE_LOG_INFO("OpenGL Info:");
    CORE_LOG_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
    CORE_LOG_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
    CORE_LOG_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));

    // IMPORTANT: This connects WindowData struct to the GLFW window instance.
    glfwSetWindowUserPointer(m_Window, &m_Data);
    SetVSync(true);


    // --- Set GLFW callbacks ---
    // These static lambdas will capture WindowData pointer and dispatch events.
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.Width = width;
        data.Height = height;

        // Create a WindowResizeEvent and send it to the callback
        WindowResizeEvent event(width, height);
        data.EventCallback(event);
        CORE_LOG_INFO("Window resized to {0}, {1}", width, height);
        });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        // Create a WindowCloseEvent and send it to the callback
        WindowCloseEvent event;
        data.EventCallback(event);
        });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action) {
        case GLFW_PRESS: {
            MouseButtonPressedEvent event(button); // Create the event
            data.EventCallback(event);             // Dispatch it
            break;
        }
        case GLFW_RELEASE: {
            MouseButtonReleasedEvent event(button);
            data.EventCallback(event);
            break;
        }
        }
        });

    // ... set up other callbacks for keyboard, mouse, etc. in the same way ...
}

void GlfwWindow::Shutdown() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void GlfwWindow::OnUpdate() {
    glfwPollEvents();
    // This is where you would swap buffers
    glfwSwapBuffers(m_Window); 
}

void GlfwWindow::SetVSync(bool enabled) {
    if (enabled) {
        glfwSwapInterval(1);
    }
    else {
        glfwSwapInterval(0);
    }
    m_Data.VSync = enabled;
}

bool GlfwWindow::IsVSync() const {
    return m_Data.VSync;
}