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

std::string GlfwInput::GetKeyNameImpl(int keycode) const {
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