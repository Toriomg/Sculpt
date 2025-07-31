#pragma once
#include "Platform/System/Input/Input.h"
#include "Platform/System/Window/Window.h"
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

private:
    GLFWwindow* m_NativeWindow;
};