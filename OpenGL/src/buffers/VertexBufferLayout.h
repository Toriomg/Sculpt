#pragma once

#include <vector>
#include <GL/glew.h>

#include "../Renderer.h"

struct VertexBufferElement {
	unsigned int type; // Data type (e.g., GL_FLOAT)
	unsigned int count; // Number of elements
	unsigned char normalized; // Whether the data should be normalized

	static unsigned int GetSizeOfType(unsigned int type) {
		switch (type) {
			case GL_FLOAT:         return sizeof(float);
			case GL_UNSIGNED_INT:  return sizeof(unsigned int);
			case GL_UNSIGNED_BYTE: return sizeof(unsigned char);
			default:               return 0; // Unsupported type
		}
		ASSERT(false);
		return 0; // Should never reach here
	}
};

class VertexBufferLayout {
private:
	std::vector<VertexBufferElement> m_Elements;
	unsigned int m_Stride = 0; // Total size of one vertex in bytes
public:
	VertexBufferLayout()
		: m_Stride(0) {}

    template<typename T>
    void Push(unsigned int count) {
        static_assert(std::is_same<T, void>::value, "Unsupported type");
    }

    // Explicit specializations (now inside class, compiler-specific)
    template<>
    void Push<float>(unsigned int count) {
        m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
        m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
    }

    template<>
    void Push<unsigned int>(unsigned int count) {
        m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
        m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
    }

    template<>
    void Push<unsigned char>(unsigned int count) {
        m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
        m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
    }

	inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
    inline unsigned int GetStride() const { return m_Stride; }
};