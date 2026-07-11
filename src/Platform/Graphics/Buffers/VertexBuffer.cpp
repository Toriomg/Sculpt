#include "VertexBuffer.hpp"


VertexBuffer::VertexBuffer(const void* data, unsigned int size, bool static_draw)
{
	GLCall(glGenBuffers(1, &m_RendererID));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, static_draw == true ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
	GLCall((glBindBuffer(GL_ARRAY_BUFFER, m_RendererID)));
}

void VertexBuffer::Unbind() const
{
	GLCall((glBindBuffer(GL_ARRAY_BUFFER, 0)));
}

void VertexBuffer::SetData(const void* data, unsigned int size, unsigned int offset) const
{
	this->Bind();
	GLCall((glBufferSubData(GL_ARRAY_BUFFER, offset, size, data)));
}