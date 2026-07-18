#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/glhead.hpp"
#include "Platform/Graphics/Buffers/VertexArray.hpp"
#include "Platform/Graphics/Buffers/VertexBuffer.hpp"
#include "Platform/Graphics/Buffers/VertexBufferLayout.hpp"
#include "Platform/Graphics/RenderCommand.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Renderer.hpp"

#include <algorithm>
#include <cmath>
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

// Project a local-space position through modelMatrix then camera VP → screen pixels.
Vec2 EditModeSystem::WorldToScreen(Vec3 localPos, Matx4f const& modelMatrix) const {
    Vec4 const worldH = modelMatrix * Vec4(localPos.x, localPos.y, localPos.z, 1.0f);
    Vec4 const clip   = m_Camera.GetViewProjectionMatrix() * worldH;
    if (std::abs(clip.w) < 1e-6f) { return {-9999.0f, -9999.0f}; }
    float const nx = clip.x / clip.w;
    float const ny = clip.y / clip.w;
    return {(nx * 0.5f + 0.5f) * static_cast<float>(m_ViewportW),
            (1.0f - (ny * 0.5f + 0.5f)) * static_cast<float>(m_ViewportH)};
}

uint32_t EditModeSystem::FindClosestVertex(uint32_t triBase, float sx, float sy,
                                           Matx4f const& model) const {
    uint32_t best  = m_EditMesh.indices[triBase];
    float bestDist = 1e30f;
    for (uint32_t k = 0; k < 3; ++k) {
        uint32_t const vi = m_EditMesh.indices[triBase + k];
        if (vi >= m_EditMesh.vertices.size()) { continue; }
        Vec2 const s  = WorldToScreen(m_EditMesh.vertices[vi].position, model);
        float const d = std::sqrt((s.x - sx) * (s.x - sx) + (s.y - sy) * (s.y - sy));
        if (d < bestDist) {
            bestDist = d;
            best     = vi;
        }
    }
    return best;
}

uint64_t EditModeSystem::FindClosestEdge(uint32_t triBase, float sx, float sy,
                                         Matx4f const& model) const {
    // The 3 edges of the triangle: (0,1),(1,2),(2,0).
    constexpr std::array<std::pair<uint32_t, uint32_t>, 3> kEdgeOffsets = {
      std::pair{0u, 1u},
      std::pair{1u, 2u},
      std::pair{2u, 0u}
    };

    uint64_t best  = 0;
    float bestDist = 1e30f;

    for (auto const& [oa, ob] : kEdgeOffsets) {
        uint32_t const ia = m_EditMesh.indices[triBase + oa];
        uint32_t const ib = m_EditMesh.indices[triBase + ob];
        if (ia >= m_EditMesh.vertices.size() || ib >= m_EditMesh.vertices.size()) { continue; }

        Vec2 const sa = WorldToScreen(m_EditMesh.vertices[ia].position, model);
        Vec2 const sb = WorldToScreen(m_EditMesh.vertices[ib].position, model);

        // Closest point on segment to (sx,sy).
        float const abx = sb.x - sa.x, aby = sb.y - sa.y;
        float const apx = sx - sa.x, apy = sy - sa.y;
        float const ab2 = abx * abx + aby * aby;
        float const t =
            (ab2 > 1e-12f) ? std::clamp((apx * abx + apy * aby) / ab2, 0.0f, 1.0f) : 0.0f;
        float const dx = sx - (sa.x + t * abx);
        float const dy = sy - (sa.y + t * aby);
        float const d  = std::sqrt(dx * dx + dy * dy);

        if (d < bestDist) {
            bestDist = d;
            best     = EditMesh::PackEdge(ia, ib);
        }
    }
    return best;
}

void EditModeSystem::OnMouseClick(uint32_t primitiveID, float screenX, float screenY,
                                  bool additive) {
    if (!m_EditMesh.IsActive()) { return; }
    if (!m_Scene->HasComponent<TransformComponent>(m_EditMesh.GetEntity())) { return; }

    auto const& tc     = m_Scene->GetComponent<TransformComponent>(m_EditMesh.GetEntity());
    Matx4f const model = m_GlobalTransform * tc.GetMatrix();

    // Guard against sentinel miss value (0xFFFFFFFF) or out-of-range primitive IDs.
    bool const validTri    = (primitiveID < m_EditMesh.FaceCount());
    uint32_t const triBase = validTri ? primitiveID * 3 : 0;

    if (!additive) { m_EditMesh.ClearSelection(); }

    if (validTri) {
        switch (m_EditMesh.mode) {
            case ElementMode::Vertex:
            {
                uint32_t const vi = FindClosestVertex(triBase, screenX, screenY, model);
                if (m_EditMesh.selectedVertices.contains(vi) && additive) {
                    m_EditMesh.selectedVertices.erase(vi);
                } else {
                    m_EditMesh.selectedVertices.insert(vi);
                }
                break;
            }
            case ElementMode::Edge:
            {
                uint64_t const ek = FindClosestEdge(triBase, screenX, screenY, model);
                if (m_EditMesh.selectedEdges.contains(ek) && additive) {
                    m_EditMesh.selectedEdges.erase(ek);
                } else {
                    m_EditMesh.selectedEdges.insert(ek);
                }
                break;
            }
            case ElementMode::Face:
            {
                uint32_t const fi = primitiveID;
                if (m_EditMesh.selectedFaces.contains(fi) && additive) {
                    m_EditMesh.selectedFaces.erase(fi);
                } else {
                    m_EditMesh.selectedFaces.insert(fi);
                }
                break;
            }
        }
    }

    m_EditMesh.selectionDirty = true;
    RebuildSelectionBuffer();
}

void EditModeSystem::RebuildSelectionBuffer() {
    // Collect position-only data for selected elements.
    std::vector<float> positions;  // x,y,z per vertex of selection geometry

    switch (m_EditMesh.mode) {
        case ElementMode::Vertex:
            for (uint32_t vi : m_EditMesh.selectedVertices) {
                if (vi >= m_EditMesh.vertices.size()) { continue; }
                Vec3 const& p = m_EditMesh.vertices[vi].position;
                positions.push_back(p.x);
                positions.push_back(p.y);
                positions.push_back(p.z);
            }
            m_SelectionPrimitive = GL_POINTS;
            break;

        case ElementMode::Edge:
            for (uint64_t ek : m_EditMesh.selectedEdges) {
                auto const ia = static_cast<uint32_t>(ek >> 32U);
                auto const ib = static_cast<uint32_t>(ek & 0xFFFFFFFFULL);
                if (ia >= m_EditMesh.vertices.size() || ib >= m_EditMesh.vertices.size()) {
                    continue;
                }
                Vec3 const& pa = m_EditMesh.vertices[ia].position;
                Vec3 const& pb = m_EditMesh.vertices[ib].position;
                positions.push_back(pa.x);
                positions.push_back(pa.y);
                positions.push_back(pa.z);
                positions.push_back(pb.x);
                positions.push_back(pb.y);
                positions.push_back(pb.z);
            }
            m_SelectionPrimitive = GL_LINES;
            break;

        case ElementMode::Face:
            for (uint32_t fi : m_EditMesh.selectedFaces) {
                uint32_t const triBase = fi * 3;
                if (triBase + 2 >= m_EditMesh.indices.size()) { continue; }
                for (uint32_t k = 0; k < 3; ++k) {
                    uint32_t const vi = m_EditMesh.indices[triBase + k];
                    if (vi >= m_EditMesh.vertices.size()) { continue; }
                    Vec3 const& p = m_EditMesh.vertices[vi].position;
                    positions.push_back(p.x);
                    positions.push_back(p.y);
                    positions.push_back(p.z);
                }
            }
            m_SelectionPrimitive = GL_TRIANGLES;
            break;
    }

    m_SelectionDrawCount = static_cast<uint32_t>(positions.size() / 3);

    if (m_SelectionDrawCount == 0) {
        m_SelectionVAO = nullptr;
        m_SelectionVBO = nullptr;
        return;
    }

    auto const byteSize = static_cast<uint32_t>(positions.size() * sizeof(float));
    m_SelectionVBO      = std::make_shared<VertexBuffer>(positions.data(), byteSize, false);

    VertexBufferLayout layout;
    layout.Push<float>(3);  // position only

    m_SelectionVAO = std::make_shared<VertexArray>();
    m_SelectionVAO->AddBufferPtr(m_SelectionVBO, layout);
}

void EditModeSystem::DrawOverlay(Matx4f const& globalTransform) {
    if (!m_EditMesh.IsActive()) { return; }
    if (!m_Scene->HasComponent<MeshComponent>(m_EditMesh.GetEntity())) { return; }

    auto const& mc = m_Scene->GetComponent<MeshComponent>(m_EditMesh.GetEntity());
    auto const& tc = m_Scene->GetComponent<TransformComponent>(m_EditMesh.GetEntity());
    if (!mc.MeshAsset) { return; }

    Matx4f const worldTransform = globalTransform * tc.GetMatrix();

    // Base overlay: dark edges + orange vertex dots.
    Renderer::SubmitEditOverlay(mc.MeshAsset, worldTransform);

    // Selection highlight drawn on top (depth-test off so it's always visible).
    if (m_SelectionDrawCount == 0 || !m_SelectionVAO) { return; }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    Renderer::SubmitSelectionHighlight(m_SelectionVAO, m_SelectionDrawCount, m_SelectionPrimitive,
                                       worldTransform);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_PROGRAM_POINT_SIZE);

    m_EditMesh.selectionDirty = false;
}
