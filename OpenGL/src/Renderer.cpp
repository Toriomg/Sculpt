#include "Renderer.h"
#include <iostream>

void GLClearError() {
    while (glGetError() != GL_NO_ERROR); // Clear all OpenGL errors
}

bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) { // Check for OpenGL errors
        std::cerr << "[OpenGL Error] (" << error << "): "
            << function << " " << file << ":" << line << std::endl; // Log the error
        return false; // Return false if an error occurred
    }
    return true; // Return true if no errors occurred
}