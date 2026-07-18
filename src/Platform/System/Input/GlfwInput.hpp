// GLFW-backed InputImpl: polls the native GLFWwindow for key/mouse state; injected into Input by
// Application on startup.
#pragma once
#include "Platform/System/Input/Input.hpp"

struct GLFWwindow;  // opaque GLFW handle; full type only needed in .cpp
class GlfwWindow;

class GlfwInput : public Input::InputImpl {
public:
    explicit GlfwInput(GlfwWindow* window);

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
