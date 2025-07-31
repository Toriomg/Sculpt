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

	glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event(key, false);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(key);
                data.EventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressedEvent event(key, true); // true for repeat
                data.EventCallback(event);
                break;
			}
        }
        });
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

std::string GlfwWindow::GetKeyName(int keycode) const {
    const char* name = glfwGetKeyName(keycode, 0);
    if (name) {
        return std::string(name);
    }

    switch (keycode) {
    case GLFW_KEY_SPACE:         return "Space";
    case GLFW_KEY_APOSTROPHE:    return "'";
    case GLFW_KEY_COMMA:         return ",";
    case GLFW_KEY_MINUS:         return "-";
    case GLFW_KEY_PERIOD:        return ".";
    case GLFW_KEY_SLASH:         return "/";
    case GLFW_KEY_SEMICOLON:     return ";";
    case GLFW_KEY_EQUAL:         return "=";
    case GLFW_KEY_LEFT_BRACKET:  return "[";
    case GLFW_KEY_BACKSLASH:     return "\\";
    case GLFW_KEY_RIGHT_BRACKET: return "]";
    case GLFW_KEY_GRAVE_ACCENT:  return "`";
    case GLFW_KEY_ESCAPE:        return "Escape";
    case GLFW_KEY_ENTER:         return "Enter";
    case GLFW_KEY_TAB:           return "Tab";
    case GLFW_KEY_BACKSPACE:     return "Backspace";
    case GLFW_KEY_INSERT:        return "Insert";
    case GLFW_KEY_DELETE:        return "Delete";
    case GLFW_KEY_RIGHT:         return "Right Arrow";
    case GLFW_KEY_LEFT:          return "Left Arrow";
    case GLFW_KEY_DOWN:          return "Down Arrow";
    case GLFW_KEY_UP:            return "Up Arrow";
    case GLFW_KEY_PAGE_UP:       return "Page Up";
    case GLFW_KEY_PAGE_DOWN:     return "Page Down";
    case GLFW_KEY_HOME:          return "Home";
    case GLFW_KEY_END:           return "End";
    case GLFW_KEY_CAPS_LOCK:     return "Caps Lock";
    case GLFW_KEY_SCROLL_LOCK:   return "Scroll Lock";
    case GLFW_KEY_NUM_LOCK:      return "Num Lock";
    case GLFW_KEY_PRINT_SCREEN:  return "Print Screen";
    case GLFW_KEY_PAUSE:         return "Pause";
    case GLFW_KEY_F1:            return "F1";
    case GLFW_KEY_F2:            return "F2";
    case GLFW_KEY_F3:            return "F3";
    case GLFW_KEY_F4:            return "F4";
    case GLFW_KEY_F5:            return "F5";
    case GLFW_KEY_F6:            return "F6";
    case GLFW_KEY_F7:            return "F7";
    case GLFW_KEY_F8:            return "F8";
    case GLFW_KEY_F9:            return "F9";
    case GLFW_KEY_F10:           return "F10";
    case GLFW_KEY_F11:           return "F11";
    case GLFW_KEY_F12:           return "F12";

    case GLFW_KEY_KP_0:          return "Numpad 0";
    case GLFW_KEY_KP_1:          return "Numpad 1";
        // ... and so on for the numpad
    case GLFW_KEY_KP_ENTER:      return "Numpad Enter";
    case GLFW_KEY_LEFT_SHIFT:    return "Left Shift";
    case GLFW_KEY_LEFT_CONTROL:  return "Left Control";
    case GLFW_KEY_LEFT_ALT:      return "Left Alt";
    case GLFW_KEY_LEFT_SUPER:    return "Left Super"; // Windows/Command key
    case GLFW_KEY_RIGHT_SHIFT:   return "Right Shift";
    case GLFW_KEY_RIGHT_CONTROL: return "Right Control";
    case GLFW_KEY_RIGHT_ALT:     return "Right Alt";
    case GLFW_KEY_RIGHT_SUPER:   return "Right Super";
    case GLFW_KEY_MENU:          return "Menu";

    default:                     return "Unknown Key"; // Fallback for unrecognized keys
    }
}