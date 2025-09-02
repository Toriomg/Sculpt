#include "IndexBuffer.h"


IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
	: m_Count(count) // The count of indices IS the count passed in.
{
	//CORE_LOG_ERROR(sizeof(unsigned int) == sizeof(GLuint)); // Good practice check

	glGenBuffers(1, &m_RendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	// The total size in bytes is the element count * the size of one element.
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_RendererID); // Delete the buffEr
}

void IndexBuffer::Bind() const
{
	// Bind the vertex buffer to the GL_ARRAY_BUFFER target
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void IndexBuffer::Unbind() const
{
	// Unbind the vertex buffer by binding 0 to the GL_ARRAY_BUFFER target
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
} 