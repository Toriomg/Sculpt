#pragma once
#include "Platform/CoreUtils/Log.hpp"
#include "Platform/CoreUtils/glewDbg.hpp"

class VertexBuffer {
private:
	unsigned int m_RendererID;
public:
	VertexBuffer(const void* data, unsigned int size, bool static_draw);// Constructor to create a vertex buffer with given data and size
	~VertexBuffer();// Destructor to clean up the vertex buffer

	void Bind() const;
	void Unbind() const;
	void SetData(const void* data, unsigned int size, unsigned int offset = 0) const;
};