#pragma once
#include <memory>
#include <vector>

#include "Vertex.h"
#include "../Buffers/VertexArray.h"
#include "../Buffers/VertexBuffer.h"
#include "../Buffers/IndexBuffer.h"
#include "../Buffers/VertexBufferLayout.h"

// Represents the geometry of an object ready to be rendered.
// It owns the GPU buffers (VBO, IBO, VAO).
class Mesh
{
public:
    // Constructor takes raw vertex and index data and creates the GPU buffers.
    Mesh(const Vertex* vertices, unsigned int vertexCount, const unsigned int* indices, unsigned int indexCount);
    ~Mesh();

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

    const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
    const std::vector<unsigned int>& GetIndices() const { return m_Indices; }

private:
    // We use unique_ptr because a Mesh has exclusive ownership of its buffers.
    // When a Mesh object is destroyed, these pointers will automatically
    // deallocate the GPU buffers.
    std::unique_ptr<VertexBuffer> m_VBO;
    std::unique_ptr<IndexBuffer>  m_IBO;
    std::unique_ptr<VertexArray>  m_VAO;

    // For the CPU operations
    std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
};