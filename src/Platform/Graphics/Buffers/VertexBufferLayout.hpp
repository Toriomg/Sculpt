// Describes per-vertex attribute layout (element type, count, stride); used by
// VertexArray::AddBuffer to configure attrib pointers.
#pragma once

#include <GL/glew.h>
#include <vector>

struct VertexBufferElement {
    // Represents a single element in the vertex buffer layout
    unsigned int type;         // Data type (e.g., GL_FLOAT)
    unsigned int count;        // Number of elements
    unsigned char normalized;  // Whether the data should be normalized

    static unsigned int GetSizeOfType(unsigned int type) {
        switch (type) {
            case GL_FLOAT:         return sizeof(float);
            case GL_UNSIGNED_INT:  return sizeof(unsigned int);
            case GL_UNSIGNED_BYTE: return sizeof(unsigned char);
            default:               return 0;  // Unsupported type
        }
        return 0;  // Should never reach here
    }
};

class VertexBufferLayout {
    // Represents the layout of a vertex buffer, defining how vertex data is structured
private:
    std::vector<VertexBufferElement> m_Elements;
    unsigned int m_Stride = 0;  // Total size of one vertex in bytes
public:
    VertexBufferLayout() : m_Stride(0) { }

    template <typename T> void Push(unsigned int count) {
        static_assert(sizeof(T) == 0, "Unsupported vertex attribute type");
    }

    inline std::vector<VertexBufferElement> const& GetElements() const { return m_Elements; }
    inline unsigned int GetStride() const { return m_Stride; }
};

template <> inline void VertexBufferLayout::Push<float>(unsigned int count) {
    m_Elements.push_back({GL_FLOAT, count, GL_FALSE});
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
}

template <> inline void VertexBufferLayout::Push<unsigned int>(unsigned int count) {
    m_Elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
}

template <> inline void VertexBufferLayout::Push<unsigned char>(unsigned int count) {
    m_Elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
}
