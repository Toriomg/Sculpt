#pragma once
#include <memory>

class VertexArray;
class VertexBuffer;
class IndexBuffer;

// Represents the geometry of an object ready to be rendered.
// It owns the GPU buffers (VBO, IBO, VAO).
class Mesh
{
public:
    // Constructor takes raw vertex and index data and creates the GPU buffers.
    Mesh(const void* vertices, unsigned int vertexSize, const unsigned int* indices, unsigned int indexCount);
    ~Mesh() = default;

    // Meshes are not meant to be copied, as that would duplicate GPU resources.
    // We delete the copy constructor and copy assignment operator.
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // We allow moving, which transfers ownership of the GPU resources.
    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    // Public getters for the renderer to use.
    // They return references, not pointers, for direct use.
    const VertexArray& GetVAO() const { return *m_VAO; }
    const IndexBuffer& GetIBO() const { return *m_IBO; }

private:
    // We use unique_ptr because a Mesh has exclusive ownership of its buffers.
    // When a Mesh object is destroyed, these pointers will automatically
    // deallocate the GPU buffers.
    std::unique_ptr<VertexBuffer> m_VBO;
    std::unique_ptr<IndexBuffer>  m_IBO;
    std::unique_ptr<VertexArray>  m_VAO;
};