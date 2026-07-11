// GLFW-backed InputImpl: polls the native GLFWwindow for key/mouse state; injected into Input by Application on startup.
#pragma once
#include "Platform/System/Input/Input.hpp"
#include "Platform/System/Window/Window.hpp"
#include <GLFW/glfw3.h>

class GlfwInput : public Input::InputImpl {
public:
    GlfwInput(Window* window); // Constructor takes the abstract Window

protected:
    // Implement the pure virtual functions from the base class
    virtual void OnUpdateImpl() override;
    virtual bool IsKeyPressedImpl(KeyCode key) override;
    virtual bool IsMouseButtonPressedImpl(MouseCode button) override;
    virtual Vec2 GetMousePositionImpl() override;
    std::string GetKeyNameImpl(int keycode) const override;
private:
    GLFWwindow* m_NativeWindow;
};