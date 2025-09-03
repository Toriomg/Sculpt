#include "RenderCommand.h"
#include <GL/glew.h>

void RenderCommand::Init() {
    // Enable features you'll always want, like depth testing
	CORE_LOG_CRITICAL("Initializing RenderCommand");
    glEnable(GL_DEPTH_TEST);
}

void RenderCommand::SetClearColor(const Vec4& color) {
    glClearColor(color.x, color.y, color.z, color.w);
}

void RenderCommand::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderCommand::Draw(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexBuffer>& indexBuffer) {
	//Bind the buffers before drawing
	vertexArray->Bind();
	indexBuffer->Bind();

	// Draw the elements using the index buffer
    glDrawElements(
        GL_TRIANGLES,
        indexBuffer->GetCount(),
        GL_UNSIGNED_INT,
        nullptr
    );
}

void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    glViewport(x, y, width, height);
}