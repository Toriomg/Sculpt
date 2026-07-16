// GLFW concrete Window: owns GLFWwindow*, installs GLFW callbacks, and initializes the OpenGL
// context.
#pragma once
#include "Platform/System/Window/Window.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GlfwWindow : public Window {
public:
    GlfwWindow(std::string_view title, uint32_t width, uint32_t height);
    ~GlfwWindow() override;

    void OnUpdate() override;

    uint32_t GetWidth() const override { return m_Data.Width; }
    uint32_t GetHeight() const override { return m_Data.Height; }

    void SetEventCallback(EventCallbackFn const& callback) override {
        m_Data.EventCallback = callback;
    }
    void SetVSync(bool enabled) override;
    bool IsVSync() const override;

    void* GetNativeWindow() const override { return m_Window; }

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
