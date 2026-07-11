#pragma once

#include "Platform/Graphics/Buffers/VertexArray.h"
#include "Platform/Graphics/Buffers/IndexBuffer.h"
#include "Platform/Graphics/Vertex.h"
#include "Platform/CoreUtils/Math/maths.h"
#include "AssetManager/IAsset.h"
#include "Core/glhead.h"

class Mesh : public IAsset {
public:
    Mesh(std::shared_ptr<VertexArray> va, std::shared_ptr<IndexBuffer> ib)
        : m_VertexArray(va), m_IndexBuffer(ib) {
    }


    const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VertexArray; }
    const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

	static std::shared_ptr<Mesh> CreateCube(float size);
    static std::shared_ptr<Mesh> CreatePyramid(float size);
    static std::shared_ptr<Mesh> CreateSphere(float radius, int sectors, int stacks);
    static std::shared_ptr<Mesh> CreateTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments);
    static std::shared_ptr<Mesh> CreateDodecahedron(float size);
    static std::shared_ptr<Mesh> CreateIcosahedron(float size);
    static std::shared_ptr<Mesh> CreateMeshFromData(
        const void* vertices,
        uint32_t vertexSize,
        const uint32_t* indices,
        uint32_t indexCount
    );
private:

    std::shared_ptr<VertexArray> m_VertexArray;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
};