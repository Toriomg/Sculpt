#include "Core/Systems/Commands/MeshEditCommand.hpp"

#include "Core/Components/Component.hpp"
#include "Core/EditMesh/EditModeSystem.hpp"
#include "Core/Scene.hpp"
#include "Platform/Graphics/Vertex.hpp"
#include "Renderer/Mesh.hpp"

MeshEditCommand::MeshEditCommand(Scene* scene, entt::entity entity, EditModeSystem* editSystem,
                                 std::vector<EditVertex> vertsBefore,
                                 std::vector<uint32_t> indsBefore,
                                 std::vector<EditVertex> vertsAfter,
                                 std::vector<uint32_t> indsAfter, std::string description)
    : m_Scene(scene), m_Entity(entity), m_EditSystem(editSystem), m_Before(std::move(vertsBefore)),
      m_After(std::move(vertsAfter)), m_IndsBefore(std::move(indsBefore)),
      m_IndsAfter(std::move(indsAfter)), m_Description(std::move(description)) { }

void MeshEditCommand::Apply(std::vector<EditVertex> const& verts,
                            std::vector<uint32_t> const& inds) const {
    if (!m_Scene->HasComponent<MeshComponent>(m_Entity)) { return; }
    auto& mc = m_Scene->GetComponent<MeshComponent>(m_Entity);
    if (!mc.MeshAsset) { return; }

    std::vector<Vertex> gpuVerts(verts.size());
    for (size_t i = 0; i < verts.size(); ++i) {
        gpuVerts[i].pos      = verts[i].position;
        gpuVerts[i].normal   = verts[i].normal;
        gpuVerts[i].texCoord = verts[i].texCoord;
    }
    mc.MeshAsset->UpdateData(gpuVerts, inds);

    if (m_EditSystem != nullptr &&
        m_EditSystem->IsActive() &&
        m_EditSystem->GetEditedEntity() == m_Entity)
    {
        m_EditSystem->SyncFromVertices(verts, inds);
    }
}

void MeshEditCommand::Execute() {
    Apply(m_After, m_IndsAfter);
}
void MeshEditCommand::Undo() {
    Apply(m_Before, m_IndsBefore);
}
