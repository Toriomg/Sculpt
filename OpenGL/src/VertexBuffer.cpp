#include "VertexBuffer.h"
#include "Renderer.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
	GLCall(glGenBuffers(1, &m_RendererID)); // Generate a buffer ID
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID)); // Bind the buffer to the GL_ARRAY_BUFFER target
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW)); // Upload the vertex data to the buffer
}

VertexBuffer::~VertexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID)); // Delete the buffer
}

void VertexBuffer::Bind() const
{
	// Bind the vertex buffer to the GL_ARRAY_BUFFER target
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const
{
	// Unbind the vertex buffer by binding 0 to the GL_ARRAY_BUFFER target
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}