#pragma once


class IndexBuffer {
private:
	unsigned int m_RendererID;
	unsigned int m_Count;
public:
	IndexBuffer(const unsigned int* data, unsigned int count);// Constructor to create a Index buffer with given data and size
	~IndexBuffer();// Destructor to clean up the Index buffer

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() const { return m_Count; } // Returns the count of indices in the buffer
};