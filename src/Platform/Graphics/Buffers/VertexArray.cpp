#include "VertexArray.hpp"

VertexArray::VertexArray() {
    GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray() {
    GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(VertexBuffer const& vb, VertexBufferLayout const& layout) const {
    Bind();
    vb.Bind();
    auto const& elements = layout.GetElements();
    unsigned int offset  = 0;
    for (unsigned int i = 0; i < elements.size(); i++) {
        auto const& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        // glVertexAttribPointer takes a void* offset, not an integer.
        // Casting through uintptr_t is the standard-compliant way to convert a byte
        // offset to a pointer without triggering UB from integer-to-pointer casts.
        GLCall(glVertexAttribPointer(
            i, element.count, element.type, element.normalized, layout.GetStride(),
            reinterpret_cast<void const*>(
                static_cast<uintptr_t>(offset))));  // NOLINT(performance-no-int-to-ptr)
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }
}

void VertexArray::AddBufferPtr(std::shared_ptr<VertexBuffer> const& vb,
                               VertexBufferLayout const& layout) {
    Bind();
    vb->Bind();

    auto const& elements = layout.GetElements();
    unsigned int offset  = 0;
    for (unsigned int i = 0; i < elements.size(); i++) {
        auto const& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(
            i, element.count, element.type, element.normalized, layout.GetStride(),
            reinterpret_cast<void const*>(
                static_cast<uintptr_t>(offset))));  // NOLINT(performance-no-int-to-ptr)
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }

    // Keep the VBO alive for the lifetime of this VAO; the VAO does not own the VBO in OpenGL.
    m_VertexBuffers.push_back(vb);
}

void VertexArray::Bind() const {
    GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() {
    GLCall(glBindVertexArray(0));
}
