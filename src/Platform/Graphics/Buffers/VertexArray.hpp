// OpenGL VAO: bundles one or more VertexBuffers with their attribute layouts; owned by Mesh.
#pragma once

#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"
#include <memory>
#include <vector>

class VertexBufferLayout;

class VertexArray {
    // A VertexArray is an OpenGL object that encapsulates the vertex buffer and its layout.
public:
    VertexArray();
    ~VertexArray();

    void AddBuffer(VertexBuffer const& vb, VertexBufferLayout const& layout) const;

    void AddBufferPtr(std::shared_ptr<VertexBuffer> const& vb, VertexBufferLayout const& layout);

    void Bind() const;
    static void Unbind() ;

private:
    unsigned int m_RendererID{};
    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
};
