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

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const {
    shader.Bind(); // Bind the shader program
    va.Bind(); // Bind the Vertex Array Object (VAO)
    ib.Bind(); // Bind the Index Buffer Object (IBO)

    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::Clear() const {
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // Clear the color and depth buffers
}