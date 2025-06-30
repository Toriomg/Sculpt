#pragma once
#include "GLFW/glfw3.h" // We need GLFW for the window handle

// Define the structure that holds the mouse state.
// This is the "blueprint" for the variable.
struct MouseState {
    double lastX = 0.0;
    double lastY = 0.0;
    float x_offset = 0.0f;
    float y_offset = 0.0f;
    bool firstMouse = true;
};

extern MouseState g_MouseState;

void InitializeInput(GLFWwindow* window);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);