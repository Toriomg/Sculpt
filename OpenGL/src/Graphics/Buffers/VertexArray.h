#pragma once

#include <vector>
#include <memory>
#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray {
	// A VertexArray is an OpenGL object that encapsulates the vertex buffer and its layout.
private:
	unsigned int m_RendererID;
	std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void AddBufferPtr(const std::shared_ptr<VertexBuffer>& vb, const VertexBufferLayout& layout);

	void Bind() const;
	void Unbind() const;
};