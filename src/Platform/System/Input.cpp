// In: Platform/System/Input.cpp

#include "Input.h"

void InputManager::Init(GLFWwindow* window) {
    m_Window = window;
    // Set the static callback functions
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSetCursorPosCallback(m_Window, cursor_position_callback);
}

// This is the key to making "IsPressed" work.
// It should be called once per frame, before any input is processed.
void InputManager::Update() {
    // Copy the current frame's state to the last frame's state
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
        m_LastMouseButtonState[i] = m_MouseButtonState[i];
    }

    // Reset mouse delta
    m_MouseDeltaX = 0.0;
    m_MouseDeltaY = 0.0;
}

// --- Queries ---
bool InputManager::IsMouseButtonDown(int button) {
    if (button > GLFW_MOUSE_BUTTON_LAST) return false;
    return m_MouseButtonState[button];
}

bool InputManager::IsMouseButtonPressed(int button) {
    if (button > GLFW_MOUSE_BUTTON_LAST) return false;
    // Was down this frame, but not last frame
    return m_MouseButtonState[button] && !m_LastMouseButtonState[button];
}

double InputManager::GetMouseX() { return m_CurrentMouseX; }
double InputManager::GetMouseY() { return m_CurrentMouseY; }
double InputManager::GetMouseDeltaX() { return m_MouseDeltaX; }
double InputManager::GetMouseDeltaY() { return m_MouseDeltaY; }


// --- Static GLFW Callbacks ---
// These static functions get the singleton instance and update its state.

void InputManager::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    InputManager& instance = InputManager::Get();
    if (button <= GLFW_MOUSE_BUTTON_LAST) {
        if (action == GLFW_PRESS) {
            instance.m_MouseButtonState[button] = true;
        }
        else if (action == GLFW_RELEASE) {
            instance.m_MouseButtonState[button] = false;
        }
    }
}

void InputManager::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    InputManager& instance = InputManager::Get();

    if (instance.m_FirstMouse) {
        instance.m_LastMouseX = xpos;
        instance.m_LastMouseY = ypos;
        instance.m_FirstMouse = false;
    }

    instance.m_LastMouseX = instance.m_CurrentMouseX;
    instance.m_LastMouseY = instance.m_CurrentMouseY;
    instance.m_CurrentMouseX = xpos;
    instance.m_CurrentMouseY = ypos;

    // Calculate delta after updating positions
    instance.m_MouseDeltaX = instance.m_CurrentMouseX - instance.m_LastMouseX;
    // Note: Y-axis is often inverted in screen vs world coordinates.
    // You might want to flip this sign depending on your camera system.
    instance.m_MouseDeltaY = instance.m_LastMouseY - instance.m_CurrentMouseY;
}