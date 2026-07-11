#include "GlfwWindow.h"
#include "Platform/CoreUtils/Log.h"

std::unique_ptr<Window> Window::Create(std::string_view title, uint32_t width, uint32_t height) {
    return std::make_unique<GlfwWindow>(title, width, height);
}

GlfwWindow::GlfwWindow(std::string_view title, uint32_t width, uint32_t height) {
    Init(title, width, height);
}

GlfwWindow::~GlfwWindow() {
    Shutdown();
}

void GlfwWindow::Init(std::string_view title, uint32_t width, uint32_t height) {
    m_Data.Title  = std::string(title);
    m_Data.Width  = width;
    m_Data.Height = height;

    CORE_LOG_INFO("Creating window {0} ({1}, {2})", title, width, height);

    if (!glfwInit()) {
        CORE_LOG_CRITICAL("ERROR: glfwInit() failed.");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    m_Window = glfwCreateWindow(
        static_cast<int>(width), static_cast<int>(height),
        m_Data.Title.c_str(), nullptr, nullptr
    );
    if (!m_Window) {
        glfwTerminate();
        CORE_LOG_CRITICAL("ERROR: glfwCreateWindow() failed.");
        return;
    }

    glfwMakeContextCurrent(m_Window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        CORE_LOG_CRITICAL("FATAL: Failed to initialize GLEW!");
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    CORE_LOG_INFO("GLEW Initialized");
    CORE_LOG_INFO("OpenGL Info:");
    CORE_LOG_INFO("  Vendor: {0}",   reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    CORE_LOG_INFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    CORE_LOG_INFO("  Version: {0}",  reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    glfwSetWindowUserPointer(m_Window, &m_Data);
    SetVSync(true);

    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int w, int h) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        data.Width  = static_cast<uint32_t>(w);
        data.Height = static_cast<uint32_t>(h);
        WindowResizeEvent event(data.Width, data.Height);
        data.EventCallback(event);
        CORE_LOG_INFO("Window resized to {0}, {1}", w, h);
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        data.EventCallback(event);
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int /*mods*/) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        switch (action) {
            case GLFW_PRESS:   { MouseButtonPressedEvent  e(button); data.EventCallback(e); break; }
            case GLFW_RELEASE: { MouseButtonReleasedEvent e(button); data.EventCallback(e); break; }
        }
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
        data.EventCallback(event);
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        switch (action) {
            case GLFW_PRESS:   { KeyPressedEvent  e(key, false); data.EventCallback(e); break; }
            case GLFW_RELEASE: { KeyReleasedEvent e(key);        data.EventCallback(e); break; }
            case GLFW_REPEAT:  { KeyPressedEvent  e(key, true);  data.EventCallback(e); break; }
        }
    });
}

void GlfwWindow::Shutdown() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void GlfwWindow::OnUpdate() {
    glfwPollEvents();
    glfwSwapBuffers(m_Window);
}

void GlfwWindow::SetVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
    m_Data.VSync = enabled;
}

bool GlfwWindow::IsVSync() const {
    return m_Data.VSync;
}
