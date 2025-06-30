#include "InputManager.h"

MouseState g_MouseState;


// Nota: Es una función normal, no necesita ser parte de una clase para funcionar como callback.
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (g_MouseState.firstMouse) {
        g_MouseState.lastX = xpos;
        g_MouseState.lastY = ypos;
        g_MouseState.firstMouse = false;
    }

    g_MouseState.x_offset = static_cast<float>(xpos - g_MouseState.lastX);
    g_MouseState.y_offset = static_cast<float>(g_MouseState.lastY - ypos); // Invertido

    g_MouseState.lastX = xpos;
    g_MouseState.lastY = ypos;
}


void InitializeInput(GLFWwindow* window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    g_MouseState.lastX = width / 2.0;
    g_MouseState.lastY = height / 2.0;

    // Configura el modo del cursor y registra el callback.
    // Toda la lógica de inicialización de input está ahora aquí.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
}