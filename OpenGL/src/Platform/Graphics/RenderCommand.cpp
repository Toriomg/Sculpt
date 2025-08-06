#include "RenderCommand.h"
#include <GL/glew.h>

void RenderCommand::Init() {
    // Enable features you'll always want, like depth testing
    glEnable(GL_DEPTH_TEST);
}

void RenderCommand::SetClearColor(const Vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void RenderCommand::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderCommand::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) {
    glDrawElements(
        GL_TRIANGLES,
        vertexArray->GetIndexBuffer()->GetCount(),
        GL_UNSIGNED_INT,
        nullptr
    );
}

void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    glViewport(x, y, width, height);
}