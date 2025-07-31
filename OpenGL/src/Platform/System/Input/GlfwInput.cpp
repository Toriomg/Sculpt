#include "GlfwInput.h"



GlfwInput::GlfwInput(Window* window) {
    // We need the native GLFW window handle for polling
    m_NativeWindow = static_cast<GLFWwindow*>(window->GetNativeWindow());
}

void GlfwInput::OnUpdateImpl() {
    // This can be used for more advanced features later, like
    // tracking "just pressed" keys. For now, it's empty.
}

bool GlfwInput::IsKeyPressedImpl(KeyCode key) {
    auto state = glfwGetKey(m_NativeWindow, static_cast<int>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool GlfwInput::IsMouseButtonPressedImpl(MouseCode button) {
    auto state = glfwGetMouseButton(m_NativeWindow, static_cast<int>(button));
    return state == GLFW_PRESS;
}

Vec2 GlfwInput::GetMousePositionImpl() {
    double xpos, ypos;
    glfwGetCursorPos(m_NativeWindow, &xpos, &ypos);
    return Vec2((float)xpos, (float)ypos);
}