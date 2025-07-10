#pragma once

#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray {
	// A VertexArray is an OpenGL object that encapsulates the vertex buffer and its layout.
private:
public:
	VertexArray();
	unsigned int m_RendererID;
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void Bind() const;
	void Unbind() const;
};