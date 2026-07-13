#include "RenderCommand.hpp"
#include "Platform/CoreUtils/glewDbg.hpp"

void RenderCommand::Init() {
	CORE_LOG_INFO("Initializing RenderCommand");
    GLCall(glEnable(GL_DEPTH_TEST));
    // Required for gl_PointSize to take effect in vertex shaders
    GLCall(glEnable(GL_PROGRAM_POINT_SIZE));
}

void RenderCommand::SetClearColor(const Vec4& color) {
    GLCall(glClearColor(color.x, color.y, color.z, color.w));
}

void RenderCommand::Clear() {
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void RenderCommand::Draw(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexBuffer>& indexBuffer) {
	vertexArray->Bind();
	indexBuffer->Bind();
    GLCall(glDrawElements(GL_TRIANGLES, indexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr));
}

void RenderCommand::DrawPoints(const std::shared_ptr<VertexArray>& vertexArray, uint32_t vertexCount)
{
    vertexArray->Bind();
    GLCall(glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertexCount)));
}

void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    GLCall(glViewport(x, y, width, height));
}