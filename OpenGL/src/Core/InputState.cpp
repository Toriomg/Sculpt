#include "InputManager.h"

MouseState g_MouseState;

void ResetMouseOffset() {
    g_MouseState.x_offset = 0.0f;
    g_MouseState.y_offset = 0.0f;
}

void ResetMouseClicks() {
    g_MouseState.leftButtonPressed = false;
    g_MouseState.rightButtonPressed = false;
}

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);
    if (GLFW_MOUSE_BUTTON_LEFT){
        g_MouseState.leftButtonPressed = true;
	}
    if (GLFW_MOUSE_BUTTON_RIGHT){
        g_MouseState.rightButtonPressed = true;
    }
	g_MouseState.firstMouse = true; // Para que la primera posición del mouse sea considerada como el centro   
}