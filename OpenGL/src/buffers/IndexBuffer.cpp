#include "IndexBuffer.h"
#include "../Renderer.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
	: m_Count(count) // Initialize the count of indices
{
	GLCall(glGenBuffers(1, &m_RendererID)); // Generate a buffer ID
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID)); // Bind the buffer to the GL_ARRAY_BUFFER target
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data, GL_STATIC_DRAW)); // Upload the vertex data to the buffer
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