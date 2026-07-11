#include "VertexBuffer.hpp"


VertexBuffer::VertexBuffer(const void* data, unsigned int size, bool static_draw)
{
	GLCall(glGenBuffers(1, &m_RendererID)); // Generate a buffer ID
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID)); // Bind the buffer to the GL_ARRAY_BUFFER target
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, static_draw == true ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW)); // Upload the vertex data to the buffer
}

VertexBuffer::~VertexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID)); // Delete the buffer
}

void VertexBuffer::Bind() const
{
	// Bind the vertex buffer to the GL_ARRAY_BUFFER target
	GLCall((glBindBuffer(GL_ARRAY_BUFFER, m_RendererID)));
}

void VertexBuffer::Unbind() const
{
	// Unbind the vertex buffer by binding 0 to the GL_ARRAY_BUFFER target
	GLCall((glBindBuffer(GL_ARRAY_BUFFER, 0)));
}

void VertexBuffer::SetData(const void* data, unsigned int size, unsigned int offset) const
{
	// Bind the vertex buffer and upload new data to it
	this->Bind();
	GLCall((glBufferSubData(GL_ARRAY_BUFFER, offset, size, data)));
}