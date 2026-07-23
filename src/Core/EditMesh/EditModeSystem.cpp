#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Platform/Graphics/Vertex.hpp"
#include "Renderer/Mesh.hpp"

#include <vector>

EditModeSystem::EditModeSystem(Scene* scene, Camera const& camera)
    : m_Scene(scene), m_Camera(camera) { }

void EditModeSystem::Enter(entt::entity entity) {
    if (entity == entt::null || !m_Scene->HasComponent<MeshComponent>(entity)) { return; }

    m_EditMesh.Clear();
    m_EditMesh.SetEntity(entity);

    auto const& mc = m_Scene->GetComponent<MeshComponent>(entity);
    if (!mc.MeshAsset) { return; }

    auto const& cpuVerts   = mc.MeshAsset->GetCpuVertices();
    auto const& cpuIndices = mc.MeshAsset->GetCpuIndices();

    if (!cpuVerts.empty()) {
        m_EditMesh.vertices.resize(cpuVerts.size());
        for (size_t i = 0; i < cpuVerts.size(); ++i) {
            m_EditMesh.vertices[i].position = cpuVerts[i].pos;
            m_EditMesh.vertices[i].normal   = cpuVerts[i].normal;
            m_EditMesh.vertices[i].texCoord = cpuVerts[i].texCoord;
        }
        m_EditMesh.indices = cpuIndices;
    } else {
        // Fallback for meshes without CPU data (raw float-array factories).
        uint32_t const vertexCount = mc.MeshAsset->GetVertexCount();
        m_EditMesh.vertices.resize(vertexCount);
        uint32_t const indexCount = mc.MeshAsset->GetIndexBuffer()->GetCount();
        m_EditMesh.indices.resize(indexCount);
        for (uint32_t i = 0; i < indexCount; ++i) { m_EditMesh.indices[i] = i % vertexCount; }
    }

    m_SelectionVAO       = nullptr;
    m_SelectionVBO       = nullptr;
    m_SelectionDrawCount = 0;
}

void EditModeSystem::Exit() {
    m_EditMesh.Clear();
    m_SelectionVAO       = nullptr;
    m_SelectionVBO       = nullptr;
    m_SelectionDrawCount = 0;
}

void EditModeSystem::SetViewportSize(uint32_t w, uint32_t h) {
    m_ViewportW = w;
    m_ViewportH = h;
}

void EditModeSystem::SetElementMode(ElementMode mode) {
    m_EditMesh.mode = mode;
    m_EditMesh.ClearSelection();
    RebuildSelectionBuffer();
}

// ---------------------------------------------------------------------------
// GPU flush
// ---------------------------------------------------------------------------

void EditModeSystem::FlushToGPU(bool fullRebuild) {
    if (!m_EditMesh.IsActive()) { return; }
    auto& mc = m_Scene->GetComponent<MeshComponent>(m_EditMesh.GetEntity());
    if (!mc.MeshAsset) { return; }

    std::vector<Vertex> gpuVerts(m_EditMesh.vertices.size());
    for (size_t i = 0; i < m_EditMesh.vertices.size(); ++i) {
        gpuVerts[i].pos      = m_EditMesh.vertices[i].position;
        gpuVerts[i].normal   = m_EditMesh.vertices[i].normal;
        gpuVerts[i].texCoord = m_EditMesh.vertices[i].texCoord;
    }

    if (fullRebuild) {
        mc.MeshAsset->UpdateData(gpuVerts, m_EditMesh.indices);
    } else {
        mc.MeshAsset->UpdateVertices(gpuVerts);
    }
}

void EditModeSystem::SyncFromVertices(std::vector<EditVertex> const& verts,
                                      std::vector<uint32_t> const& inds) {
    m_EditMesh.vertices = verts;
    m_EditMesh.indices  = inds;
    m_EditMesh.ClearSelection();
    m_ExtrudeState.reset();
    RebuildSelectionBuffer();
}
