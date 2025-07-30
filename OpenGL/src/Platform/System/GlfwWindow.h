#pragma once
#include "Platform/System/Window.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GlfwWindow : public Window {
public:
    GlfwWindow(const std::string& title, unsigned int width, unsigned int height);
    virtual ~GlfwWindow();

    void OnUpdate() override;

    unsigned int GetWidth() const override { return m_Data.Width; }
    unsigned int GetHeight() const override { return m_Data.Height; }

    void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
    void SetVSync(bool enabled) override;
    bool IsVSync() const override;

    void* GetNativeWindow() const override { return m_Window; }

private:
    void Init(const std::string& title, unsigned int width, unsigned int height);
    void Shutdown();

    GLFWwindow* m_Window;

    // A struct to hold all our windows data.
    // Useful for passing data to GLFWs C style callbacks.
    struct WindowData {
        std::string Title;
        unsigned int Width, Height;
        bool VSync;
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};
