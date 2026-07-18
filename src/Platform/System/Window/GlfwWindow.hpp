// GLFW window: owns GLFWwindow*, installs GLFW callbacks, and initializes the OpenGL context.
#pragma once
#include "Core/glhead.hpp"
#include "Platform/System/Events/KeyboardEvent.hpp"
#include "Platform/System/Events/MouseEvent.hpp"
#include "Platform/System/Events/WindowEvent.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <string_view>

class GlfwWindow {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    GlfwWindow(std::string_view title, uint32_t width, uint32_t height);
    ~GlfwWindow();
    GlfwWindow(GlfwWindow const&)            = delete;
    GlfwWindow& operator=(GlfwWindow const&) = delete;
    GlfwWindow(GlfwWindow&&)                 = delete;
    GlfwWindow& operator=(GlfwWindow&&)      = delete;

    void OnUpdate();

    uint32_t GetWidth() const { return m_Data.Width; }
    uint32_t GetHeight() const { return m_Data.Height; }

    void SetEventCallback(EventCallbackFn const& callback) { m_Data.EventCallback = callback; }
    void SetVSync(bool enabled);
    bool IsVSync() const;

    void* GetNativeWindow() const { return m_Window; }

private:
    void Init(std::string_view title, uint32_t width, uint32_t height);
    void Shutdown();

    GLFWwindow* m_Window{};

    struct WindowData {
        std::string Title;
        uint32_t Width, Height;
        bool VSync;
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};
