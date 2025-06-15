#pragma once

class VertexBuffer {
private:
	unsigned int m_RendererID;
public:
	VertexBuffer(const void* data, unsigned int size);// Constructor to create a vertex buffer with given data and size
	~VertexBuffer();// Destructor to clean up the vertex buffer

	void Bind() const;
	void Unbind() const;
};