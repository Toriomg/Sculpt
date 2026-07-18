#include "Core/EditMesh/EditModeSystem.hpp"
#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Renderer.hpp"

void EditModeSystem::Enter(entt::entity entity) {
    if (entity == entt::null || !m_Scene->HasComponent<MeshComponent>(entity)) { return; }

    m_EditMesh.Clear();
    m_EditMesh.SetEntity(entity);

    // Mirror the GPU index buffer into the CPU EditMesh.
    // Vertex positions are needed for future mesh editing; for now this is structural.
    auto const& mc = m_Scene->GetComponent<MeshComponent>(entity);
    if (!mc.MeshAsset) { return; }

    uint32_t const vertexCount = mc.MeshAsset->GetVertexCount();
    m_EditMesh.vertices.resize(vertexCount);
    // Index data cannot be read back from GPU without a readback — populate from stored mesh count.
    // ponytail: no GPU readback; vertex positions are unavailable until Mesh stores CPU data.
    // Per-vertex selection state works on index range only for now.
    uint32_t const indexCount = mc.MeshAsset->GetIndexBuffer()->GetCount();
    m_EditMesh.indices.resize(indexCount);
    for (uint32_t i = 0; i < indexCount; ++i) { m_EditMesh.indices[i] = i % vertexCount; }
}

void EditModeSystem::Exit() {
    m_EditMesh.Clear();
}

void EditModeSystem::DrawOverlay(Matx4f const& globalTransform) const {
    if (!m_EditMesh.IsActive()) { return; }
    if (!m_Scene->HasComponent<MeshComponent>(m_EditMesh.GetEntity())) { return; }

    auto const& mc = m_Scene->GetComponent<MeshComponent>(m_EditMesh.GetEntity());
    auto const& tc = m_Scene->GetComponent<TransformComponent>(m_EditMesh.GetEntity());
    if (!mc.MeshAsset) { return; }

    Matx4f const worldTransform = globalTransform * tc.GetMatrix();
    Renderer::SubmitEditOverlay(mc.MeshAsset, worldTransform);
}
