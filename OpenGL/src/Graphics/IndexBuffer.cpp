#include "IndexBuffer.h"
#include "Renderer.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
	: m_Count(count) // The count of indices IS the count passed in.
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint)); // Good practice check

	GLCall(glGenBuffers(1, &m_RendererID));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
	// The total size in bytes is the element count * the size of one element.
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID)); // Delete the buffer
}

void IndexBuffer::Bind() const
{
	// Bind the vertex buffer to the GL_ARRAY_BUFFER target
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
}

void IndexBuffer::Unbind() const
{
	// Unbind the vertex buffer by binding 0 to the GL_ARRAY_BUFFER target
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}