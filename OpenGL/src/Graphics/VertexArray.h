#pragma once

#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray {
	// A VertexArray is an OpenGL object that encapsulates the vertex buffer and its layout.
private:
	unsigned int m_RendererID;
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void Bind() const;
	void Unbind() const;
};