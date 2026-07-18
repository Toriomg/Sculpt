// Owns a VertexArray + IndexBuffer; provides static factory methods for built-in primitives (cube,
// sphere, torus, etc.). CPU copies of vertex/index data are stored for edit-mode access.
#pragma once

#include "Platform/CoreUtils/AssetHandle.hpp"
#include "Platform/Graphics/Buffers/IndexBuffer.hpp"
#include "Platform/Graphics/Buffers/VertexArray.hpp"
#include "Platform/Graphics/Vertex.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class Mesh {
public:
    AssetHandle Handle;

    Mesh(std::shared_ptr<VertexArray> va, std::shared_ptr<IndexBuffer> ib, uint32_t vertexCount)
        : m_VertexArray(va), m_IndexBuffer(ib), m_VertexCount(vertexCount) { }

    [[nodiscard]] std::shared_ptr<VertexArray> const& GetVertexArray() const {
        return m_VertexArray;
    }
    [[nodiscard]] std::shared_ptr<IndexBuffer> const& GetIndexBuffer() const {
        return m_IndexBuffer;
    }
    [[nodiscard]] uint32_t GetVertexCount() const { return m_VertexCount; }

    // CPU-side copies populated by CreateMeshFromData; empty for meshes built with raw float
    // arrays.
    [[nodiscard]] std::vector<Vertex> const& GetCpuVertices() const { return m_CpuVertices; }
    [[nodiscard]] std::vector<uint32_t> const& GetCpuIndices() const { return m_CpuIndices; }

    static std::shared_ptr<Mesh> CreateCube(float size);
    static std::shared_ptr<Mesh> CreatePyramid(float size);
    static std::shared_ptr<Mesh> CreateSphere(float radius, int sectors, int stacks);
    static std::shared_ptr<Mesh> CreateTorus(float majorRadius, float minorRadius,
                                             int majorSegments, int minorSegments);
    static std::shared_ptr<Mesh> CreateDodecahedron(float size);
    static std::shared_ptr<Mesh> CreateIcosahedron(float size);
    // Unit arrow along +X: shaft from 0→0.65, cone tip at 1. Used by translation gizmo.
    static std::shared_ptr<Mesh> CreateArrow(int segments = 12);
    // Unit cone: base circle (radius 1) in XY plane at z=0, tip at z=+1. Used by rotation gizmo
    // arrowheads.
    static std::shared_ptr<Mesh> CreateCone(int segments = 12);
    static std::shared_ptr<Mesh> CreateMeshFromData(void const* vertices, uint32_t vertexSize,
                                                    uint32_t const* indices, uint32_t indexCount);

private:
    std::shared_ptr<VertexArray> m_VertexArray;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
    uint32_t m_VertexCount = 0;
    std::vector<Vertex> m_CpuVertices;
    std::vector<uint32_t> m_CpuIndices;
};
