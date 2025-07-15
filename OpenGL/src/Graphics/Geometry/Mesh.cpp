#include "Mesh.h"





Mesh::Mesh(const void* vertices, unsigned int vertexSize, const unsigned int* indices, unsigned int indexCount)
{
    // 1. Create the Vertex and Index Buffers on the GPU.
    //    These are now owned by this Mesh instance via unique_ptr.
    m_VBO = std::make_unique<VertexBuffer>(vertices, vertexSize, false);
    m_IBO = std::make_unique<IndexBuffer>(indices, indexCount);

    // 2. Create the Vertex Array Object.
    m_VAO = std::make_unique<VertexArray>();

    // 3. Define the layout of the vertex data.
    VertexBufferLayout layout;
    layout.Push<float>(3); // Position (x, y, z)
    layout.Push<float>(3); // texture u, v, w
    layout.Push<float>(3); // normal

    // 4. Add the Vertex Buffer and its layout to the Vertex Array Object.
    //    This configures the VAO to know how to read from the VBO.
    //    We pass *m_VBO (a reference) which is safe because m_VBO lives
    //    as long as this Mesh object.
    m_VAO->AddBuffer(*m_VBO, layout);
}

Mesh::~Mesh() = default;