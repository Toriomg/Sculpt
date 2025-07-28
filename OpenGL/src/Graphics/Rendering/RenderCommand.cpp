#include "RenderCommand.h"


void RenderCommand::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const {
    shader.Bind(); // Bind the shader program
    va.Bind(); // Bind the Vertex Array Object (VAO)
    ib.Bind(); // Bind the Index Buffer Object (IBO)

    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void RenderCommand::Clear() const {
	GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f)); // Set the clear color
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // Clear the color and depth buffers
}