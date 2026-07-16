// Legacy GLFW input singleton (InputManager); superseded by Platform/System/Input/Input.hpp.
#pragma once
#include "GLFW/glfw3.h"

class InputManager {
public:
    // Get the single instance of the InputManager
    static InputManager& Get() {
        static InputManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copies
    InputManager(InputManager const&)   = delete;
    void operator=(InputManager const&) = delete;

    // --- Public API ---
    void Init(GLFWwindow* window);
    void Update();  // Call this once at the start of every frame

    // Mouse Button Queries
    bool IsMouseButtonDown(int button);     // Is the button currently held down?
    bool IsMouseButtonPressed(int button);  // Was the button pressed THIS frame?

    // Mouse Position and Movement Queries
    double GetMouseX();
    double GetMouseY();
    double GetMouseDeltaX();
    double GetMouseDeltaY();

private:
    // Private constructor to enforce the Singleton pattern
    InputManager() = default;

    // --- Internal State ---
    GLFWwindow* m_Window = nullptr;
    double m_LastMouseX = 0.0, m_LastMouseY = 0.0;
    double m_CurrentMouseX = 0.0, m_CurrentMouseY = 0.0;
    double m_MouseDeltaX = 0.0, m_MouseDeltaY = 0.0;

    // To track "pressed this frame" vs "held down"
    bool m_MouseButtonState[GLFW_MOUSE_BUTTON_LAST + 1]     = {false};
    bool m_LastMouseButtonState[GLFW_MOUSE_BUTTON_LAST + 1] = {false};
    bool m_FirstMouse                                       = true;

    // --- GLFW Callbacks (must be static) ---
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
};
