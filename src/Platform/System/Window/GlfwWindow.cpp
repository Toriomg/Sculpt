#include "GlfwWindow.hpp"

#include "Platform/CoreUtils/Log.hpp"
#include <utility>

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

    if (glfwInit() == 0) {
        CORE_LOG_CRITICAL("ERROR: glfwInit() failed.");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    m_Window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height),
                                m_Data.Title.c_str(), nullptr, nullptr);
    if (m_Window == nullptr) {
        glfwTerminate();
        CORE_LOG_CRITICAL("ERROR: glfwCreateWindow() failed.");
        return;
    }

    glfwMakeContextCurrent(m_Window);

    // Register all GLFW callbacks before GLEW init — they are independent of OpenGL.
    glfwSetWindowUserPointer(m_Window, &m_Data);
    SetVSync(true);

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        data.EventCallback(event);
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int w, int h) {
        auto& data  = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        data.Width  = static_cast<uint32_t>(w);
        data.Height = static_cast<uint32_t>(h);
        WindowResizeEvent event(data.Width, data.Height);
        data.EventCallback(event);
    });

    glfwSetMouseButtonCallback(
        m_Window, [](GLFWwindow* window, int button, int action, int /*mods*/) {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            switch (action) {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent e(button);
                    data.EventCallback(e);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent e(button);
                    data.EventCallback(e);
                    break;
                }
                default: break;
            }
        });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
        data.EventCallback(event);
    });

    glfwSetKeyCallback(m_Window,
                       [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
                           auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
                           switch (action) {
                               case GLFW_PRESS:
                               {
                                   KeyPressedEvent e(key, false);
                                   data.EventCallback(e);
                                   break;
                               }
                               case GLFW_RELEASE:
                               {
                                   KeyReleasedEvent e(key);
                                   data.EventCallback(e);
                                   break;
                               }
                               case GLFW_REPEAT:
                               {
                                   KeyPressedEvent e(key, true);
                                   data.EventCallback(e);
                                   break;
                               }
                               default: break;
                           }
                       });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
        data.EventCallback(event);
    });

    // On HiDPI displays the framebuffer can be larger than the window in screen coordinates;
    // reading it back ensures m_Data reflects the actual pixel dimensions from the start.
    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(m_Window, &fbWidth, &fbHeight);
    m_Data.Width  = static_cast<uint32_t>(fbWidth);
    m_Data.Height = static_cast<uint32_t>(fbHeight);

    // GLEW init — on some Linux configurations (XWayland, Mesa) this returns an error
    // but OpenGL functions are still available. Treat as non-fatal.
    // glewExperimental must be set before glewInit to expose core-profile functions
    // that GLEW would otherwise skip because they lack wgl/glX extension strings.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        CORE_LOG_WARN("glewInit() returned an error — OpenGL may still be available.");
    }

    glEnable(GL_DEPTH_TEST);

    CORE_LOG_INFO("OpenGL Info:");
    CORE_LOG_INFO("  Vendor: {0}", reinterpret_cast<char const*>(glGetString(GL_VENDOR)));
    CORE_LOG_INFO("  Renderer: {0}", reinterpret_cast<char const*>(glGetString(GL_RENDERER)));
    CORE_LOG_INFO("  Version: {0}", reinterpret_cast<char const*>(glGetString(GL_VERSION)));
}

void GlfwWindow::Shutdown() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void GlfwWindow::OnUpdate() {
    glfwPollEvents();
    glfwSwapBuffers(m_Window);

    // Wayland compositors do not always fire the GLFW framebuffer-size callback reliably,
    // so we poll the actual size each frame and synthesize the event when it changes.
    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(m_Window, &fbWidth, &fbHeight);
    if (std::cmp_not_equal(fbWidth, m_Data.Width) || std::cmp_not_equal(fbHeight, m_Data.Height)) {
        m_Data.Width  = static_cast<uint32_t>(fbWidth);
        m_Data.Height = static_cast<uint32_t>(fbHeight);
        if (m_Data.EventCallback) {
            WindowResizeEvent event(m_Data.Width, m_Data.Height);
            m_Data.EventCallback(event);
        }
    }
}

void GlfwWindow::SetVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
    m_Data.VSync = enabled;
}

bool GlfwWindow::IsVSync() const {
    return m_Data.VSync;
}
