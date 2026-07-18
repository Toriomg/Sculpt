#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/glhead.hpp"
#include "Platform/Graphics/Buffers/VertexArray.hpp"
#include "Platform/Graphics/Buffers/VertexBuffer.hpp"
#include "Platform/Graphics/Buffers/VertexBufferLayout.hpp"
#include "Platform/Graphics/RenderCommand.hpp"
#include "Platform/Graphics/Vertex.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Renderer.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>
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

// ---------------------------------------------------------------------------
// Extrude — face mode
// ---------------------------------------------------------------------------

void EditModeSystem::DoExtrudeFaces(ExtrudeState& state) {
    auto& verts          = m_EditMesh.vertices;
    auto& indices        = m_EditMesh.indices;
    auto const& selFaces = m_EditMesh.selectedFaces;
    if (selFaces.empty()) { return; }

    // Averaged face normal (area-weighted so large faces dominate).
    Vec3 avgNormal{0.0f, 0.0f, 0.0f};
    for (uint32_t fi : selFaces) {
        uint32_t const base = fi * 3;
        if (base + 2 >= indices.size()) { continue; }
        Vec3 const e1 = verts[indices[base + 1]].position - verts[indices[base]].position;
        Vec3 const e2 = verts[indices[base + 2]].position - verts[indices[base]].position;
        avgNormal     = avgNormal + e1.crossProduct(e2);
    }
    float const len = std::sqrt(
        avgNormal.x * avgNormal.x + avgNormal.y * avgNormal.y + avgNormal.z * avgNormal.z);
    state.normal = (len > 1e-8f) ? avgNormal / len : Vec3{0.0f, 1.0f, 0.0f};

    // Capture original vertex indices for each selected face before any remapping.
    struct OrigFace {
        uint32_t fi;
        uint32_t v[3];
    };
    std::vector<OrigFace> origFaces;
    origFaces.reserve(selFaces.size());
    for (uint32_t fi : selFaces) {
        uint32_t const base = fi * 3;
        if (base + 2 >= indices.size()) { continue; }
        origFaces.push_back({
          fi, {indices[base], indices[base + 1], indices[base + 2]}
        });
    }

    // Duplicate each unique vertex used by the selected faces.
    std::unordered_map<uint32_t, uint32_t> oldToNew;
    for (auto const& of : origFaces) {
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const vi = of.v[k];
            if (!oldToNew.contains(vi)) {
                uint32_t const newIdx = static_cast<uint32_t>(verts.size());
                oldToNew[vi]          = newIdx;
                verts.push_back(verts[vi]);
                state.grabbedVerts.push_back(newIdx);
                state.basePositions.push_back(verts[vi].position);
            }
        }
    }

    // Remap the selected faces to the new (duplicate) vertices.
    for (auto const& of : origFaces) {
        uint32_t const base = of.fi * 3;
        indices[base]       = oldToNew[of.v[0]];
        indices[base + 1]   = oldToNew[of.v[1]];
        indices[base + 2]   = oldToNew[of.v[2]];
    }

    // Find boundary edges: edges used by exactly one selected face.
    std::unordered_map<uint64_t, int> edgeSelCount;
    for (auto const& of : origFaces) {
        for (uint32_t k = 0; k < 3; ++k) {
            edgeSelCount[EditMesh::PackEdge(of.v[k], of.v[(k + 1) % 3])]++;
        }
    }

    // Add wall quads (2 triangles each) for every boundary edge.
    // Winding: for CCW top-face edge (A→B), wall triangles (A, nA, nB) and (A, nB, B)
    // produce outward-facing wall normals.
    for (auto const& of : origFaces) {
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const A = of.v[k];
            uint32_t const B = of.v[(k + 1) % 3];
            if (edgeSelCount[EditMesh::PackEdge(A, B)] != 1) { continue; }
            uint32_t const nA = oldToNew[A];
            uint32_t const nB = oldToNew[B];
            indices.push_back(A);
            indices.push_back(nA);
            indices.push_back(nB);
            indices.push_back(A);
            indices.push_back(nB);
            indices.push_back(B);
        }
    }

    // Switch to vertex mode so the grab highlight shows the extruded vertices.
    m_EditMesh.ClearSelection();
    m_EditMesh.mode = ElementMode::Vertex;
    for (uint32_t vi : state.grabbedVerts) { m_EditMesh.selectedVertices.insert(vi); }
}

// ---------------------------------------------------------------------------
// Extrude — edge mode
// ---------------------------------------------------------------------------

void EditModeSystem::DoExtrudeEdges(ExtrudeState& state) {
    auto& verts          = m_EditMesh.vertices;
    auto& indices        = m_EditMesh.indices;
    auto const& selEdges = m_EditMesh.selectedEdges;
    if (selEdges.empty()) { return; }

    // Average normal from edge vertex normals.
    Vec3 avgNormal{0.0f, 0.0f, 0.0f};
    for (uint64_t ek : selEdges) {
        auto const ia = static_cast<uint32_t>(ek >> 32U);
        auto const ib = static_cast<uint32_t>(ek & 0xFFFFFFFFULL);
        if (ia < verts.size()) { avgNormal = avgNormal + verts[ia].normal; }
        if (ib < verts.size()) { avgNormal = avgNormal + verts[ib].normal; }
    }
    float const len = std::sqrt(
        avgNormal.x * avgNormal.x + avgNormal.y * avgNormal.y + avgNormal.z * avgNormal.z);
    state.normal = (len > 1e-8f) ? avgNormal / len : Vec3{0.0f, 1.0f, 0.0f};

    // Duplicate each unique vertex in the selected edges.
    std::unordered_map<uint32_t, uint32_t> oldToNew;
    for (uint64_t ek : selEdges) {
        auto const ia = static_cast<uint32_t>(ek >> 32U);
        auto const ib = static_cast<uint32_t>(ek & 0xFFFFFFFFULL);
        for (uint32_t vi : {ia, ib}) {
            if (vi >= verts.size() || oldToNew.contains(vi)) { continue; }
            uint32_t const newIdx = static_cast<uint32_t>(verts.size());
            oldToNew[vi]          = newIdx;
            verts.push_back(verts[vi]);
            state.grabbedVerts.push_back(newIdx);
            state.basePositions.push_back(verts[vi].position);
        }
    }

    // Add a wall quad for each selected edge: (A, nA, nB) + (A, nB, B).
    for (uint64_t ek : selEdges) {
        auto const A = static_cast<uint32_t>(ek >> 32U);
        auto const B = static_cast<uint32_t>(ek & 0xFFFFFFFFULL);
        if (!oldToNew.contains(A) || !oldToNew.contains(B)) { continue; }
        uint32_t const nA = oldToNew[A];
        uint32_t const nB = oldToNew[B];
        indices.push_back(A);
        indices.push_back(nA);
        indices.push_back(nB);
        indices.push_back(A);
        indices.push_back(nB);
        indices.push_back(B);
    }

    m_EditMesh.ClearSelection();
    m_EditMesh.mode = ElementMode::Vertex;
    for (uint32_t vi : state.grabbedVerts) { m_EditMesh.selectedVertices.insert(vi); }
}

// ---------------------------------------------------------------------------
// Extrude — vertex mode
// ---------------------------------------------------------------------------

void EditModeSystem::DoExtrudeVerts(ExtrudeState& state) {
    auto& verts          = m_EditMesh.vertices;
    auto const& selVerts = m_EditMesh.selectedVertices;
    if (selVerts.empty()) { return; }

    Vec3 avgNormal{0.0f, 0.0f, 0.0f};
    for (uint32_t vi : selVerts) {
        if (vi < verts.size()) { avgNormal = avgNormal + verts[vi].normal; }
    }
    float const len = std::sqrt(
        avgNormal.x * avgNormal.x + avgNormal.y * avgNormal.y + avgNormal.z * avgNormal.z);
    state.normal = (len > 1e-8f) ? avgNormal / len : Vec3{0.0f, 1.0f, 0.0f};

    // ponytail: vertex extrude adds orphaned verts (no edges/faces), visible only in overlay.
    // Full edge topology needs a half-edge structure.
    std::unordered_map<uint32_t, uint32_t> oldToNew;
    for (uint32_t vi : selVerts) {
        if (vi >= verts.size()) { continue; }
        uint32_t const newIdx = static_cast<uint32_t>(verts.size());
        oldToNew[vi]          = newIdx;
        verts.push_back(verts[vi]);
        state.grabbedVerts.push_back(newIdx);
        state.basePositions.push_back(verts[vi].position);
    }

    m_EditMesh.ClearSelection();
    for (uint32_t vi : state.grabbedVerts) { m_EditMesh.selectedVertices.insert(vi); }
}

// ---------------------------------------------------------------------------
// Public extrude / grab interface
// ---------------------------------------------------------------------------

void EditModeSystem::Extrude() {
    if (!m_EditMesh.IsActive() || m_ExtrudeState.has_value()) { return; }

    bool const hasSelection = !m_EditMesh.selectedFaces.empty() ||
                              !m_EditMesh.selectedEdges.empty() ||
                              !m_EditMesh.selectedVertices.empty();
    if (!hasSelection) { return; }

    ExtrudeState state;
    state.vertexCountBefore = m_EditMesh.vertices.size();
    state.indicesBefore     = m_EditMesh.indices;
    state.facesBefore       = m_EditMesh.selectedFaces;
    state.vertsBefore       = m_EditMesh.selectedVertices;
    state.edgesBefore       = m_EditMesh.selectedEdges;
    state.modeBefore        = m_EditMesh.mode;

    switch (m_EditMesh.mode) {
        case ElementMode::Face:   DoExtrudeFaces(state); break;
        case ElementMode::Edge:   DoExtrudeEdges(state); break;
        case ElementMode::Vertex: DoExtrudeVerts(state); break;
    }

    if (state.grabbedVerts.empty()) { return; }

    m_ExtrudeState = std::move(state);
    FlushToGPU(true);
    RebuildSelectionBuffer();
}

void EditModeSystem::UpdateGrab(float /*dx*/, float dy) {
    if (!m_ExtrudeState) { return; }

    // Screen Y increases downward; moving up (negative dy) extrudes outward.
    m_ExtrudeState->offset += dy * -0.005f;

    for (size_t i = 0; i < m_ExtrudeState->grabbedVerts.size(); ++i) {
        uint32_t const vi = m_ExtrudeState->grabbedVerts[i];
        if (vi >= m_EditMesh.vertices.size()) { continue; }
        m_EditMesh.vertices[vi].position =
            m_ExtrudeState->basePositions[i] + m_ExtrudeState->normal * m_ExtrudeState->offset;
    }

    FlushToGPU(false);
    RebuildSelectionBuffer();
}

void EditModeSystem::ConfirmGrab() {
    if (!m_ExtrudeState) { return; }
    m_ExtrudeState.reset();
    // GPU already reflects the final state from the last UpdateGrab call.
}

void EditModeSystem::CancelGrab() {
    if (!m_ExtrudeState) { return; }
    m_EditMesh.vertices.resize(m_ExtrudeState->vertexCountBefore);
    m_EditMesh.indices          = m_ExtrudeState->indicesBefore;
    m_EditMesh.selectedFaces    = m_ExtrudeState->facesBefore;
    m_EditMesh.selectedVertices = m_ExtrudeState->vertsBefore;
    m_EditMesh.selectedEdges    = m_ExtrudeState->edgesBefore;
    m_EditMesh.mode             = m_ExtrudeState->modeBefore;
    m_EditMesh.selectionDirty   = true;
    m_ExtrudeState.reset();
    FlushToGPU(true);
    RebuildSelectionBuffer();
}
