#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/Commands/MeshEditCommand.hpp"
#include "Core/Systems/HistorySystem.hpp"
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

    // Mesh factories produce triangle soups with no shared vertices across faces (unique vertices
    // per face for flat normals). We must therefore use quantised 3-D position as the edge
    // identity key instead of vertex indices; otherwise every edge appears exactly once and we
    // incorrectly add walls between adjacent selected faces.
    auto qf     = [](float v) { return static_cast<int32_t>(llroundf(v * 1.0e4f)); };
    using PK    = std::array<int32_t, 3>;
    auto qp     = [&qf](Vec3 const& p) -> PK { return {qf(p.x), qf(p.y), qf(p.z)}; };
    using EK    = std::pair<PK, PK>;
    auto makeEK = [&qp](Vec3 const& pa, Vec3 const& pb) -> EK {
        PK ka = qp(pa), kb = qp(pb);
        return ka < kb ? EK{ka, kb} : EK{kb, ka};
    };

    // Count how many selected faces share each geometric edge.
    // Edges with count==1 are on the boundary and need a wall quad.
    std::map<EK, int> edgePosCount;
    for (auto const& of : origFaces) {
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const a = of.v[k], b = of.v[(k + 1) % 3];
            edgePosCount[makeEK(verts[a].position, verts[b].position)]++;
        }
    }

    // Duplicate selected vertices — weld by position so faces sharing a geometric vertex
    // produce a single new top vertex rather than one per face.
    std::unordered_map<uint32_t, uint32_t> oldToNew;  // old index → new index
    std::map<PK, uint32_t> posToNew;                  // quantised pos → new index

    for (auto const& of : origFaces) {
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const vi = of.v[k];
            PK pk             = qp(verts[vi].position);
            if (!posToNew.count(pk)) {
                uint32_t const newIdx = static_cast<uint32_t>(verts.size());
                posToNew[pk]          = newIdx;
                EditVertex copy       = verts[vi];
                verts.push_back(copy);
                state.grabbedVerts.push_back(newIdx);
                state.basePositions.push_back(copy.position);
            }
            oldToNew[vi] = posToNew[pk];
        }
    }

    // Remap the selected faces to the welded new vertices.
    for (auto const& of : origFaces) {
        uint32_t const base = of.fi * 3;
        indices[base]       = oldToNew[of.v[0]];
        indices[base + 1]   = oldToNew[of.v[1]];
        indices[base + 2]   = oldToNew[of.v[2]];
    }

    // Add wall quads for every boundary edge. Use edgeUnit × extrudeNormal for the outward
    // wall normal (E × N, NOT N × E — the latter points inward for CCW-wound faces).
    for (auto const& of : origFaces) {
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const A = of.v[k];
            uint32_t const B = of.v[(k + 1) % 3];
            if (edgePosCount[makeEK(verts[A].position, verts[B].position)] != 1) { continue; }

            uint32_t const nA = oldToNew[A];
            uint32_t const nB = oldToNew[B];

            Vec3 const edgeDir = verts[B].position - verts[A].position;
            float const edgeLen =
                std::sqrt(edgeDir.x * edgeDir.x + edgeDir.y * edgeDir.y + edgeDir.z * edgeDir.z);
            Vec3 const edgeUnit   = (edgeLen > 1e-8f) ? edgeDir / edgeLen : Vec3{1.0f, 0.0f, 0.0f};
            Vec3 const wallNormal = edgeUnit.crossProduct(state.normal);
            float const wn_len    = std::sqrt(wallNormal.x * wallNormal.x +
                                              wallNormal.y * wallNormal.y +
                                              wallNormal.z * wallNormal.z);
            Vec3 const wallNUnit  = (wn_len > 1e-8f) ? wallNormal / wn_len : state.normal;

            auto makeWallVert = [&](uint32_t srcIdx, Vec3 const& wn) -> uint32_t {
                uint32_t const idx = static_cast<uint32_t>(verts.size());
                EditVertex wv      = verts[srcIdx];
                wv.normal          = wn;
                verts.push_back(wv);
                return idx;
            };
            uint32_t const wA  = makeWallVert(A, wallNUnit);
            uint32_t const wnA = makeWallVert(nA, wallNUnit);
            uint32_t const wnB = makeWallVert(nB, wallNUnit);
            uint32_t const wB  = makeWallVert(B, wallNUnit);

            indices.push_back(wA);
            indices.push_back(wnA);
            indices.push_back(wnB);
            indices.push_back(wA);
            indices.push_back(wnB);
            indices.push_back(wB);

            state.wallTopMirrors.emplace_back(wnA, nA);
            state.wallTopMirrors.emplace_back(wnB, nB);
        }
    }

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
    // Capture a copy before push_back to avoid UB from potential reallocation.
    std::unordered_map<uint32_t, uint32_t> oldToNew;
    for (uint64_t ek : selEdges) {
        auto const ia = static_cast<uint32_t>(ek >> 32U);
        auto const ib = static_cast<uint32_t>(ek & 0xFFFFFFFFULL);
        for (uint32_t vi : {ia, ib}) {
            if (vi >= verts.size() || oldToNew.contains(vi)) { continue; }
            uint32_t const newIdx = static_cast<uint32_t>(verts.size());
            oldToNew[vi]          = newIdx;
            EditVertex copy       = verts[vi];
            verts.push_back(copy);
            state.grabbedVerts.push_back(newIdx);
            state.basePositions.push_back(copy.position);
        }
    }

    // Add a wall quad for each selected edge using dedicated vertices with wall normals.
    for (uint64_t ek : selEdges) {
        auto const A = static_cast<uint32_t>(ek >> 32U);
        auto const B = static_cast<uint32_t>(ek & 0xFFFFFFFFULL);
        if (!oldToNew.contains(A) || !oldToNew.contains(B)) { continue; }
        uint32_t const nA = oldToNew[A];
        uint32_t const nB = oldToNew[B];

        Vec3 const edgeDir = verts[B].position - verts[A].position;
        float const edgeLen =
            std::sqrt(edgeDir.x * edgeDir.x + edgeDir.y * edgeDir.y + edgeDir.z * edgeDir.z);
        Vec3 const edgeUnit   = (edgeLen > 1e-8f) ? edgeDir / edgeLen : Vec3{1.0f, 0.0f, 0.0f};
        Vec3 const wallNormal = edgeUnit.crossProduct(state.normal);
        float const wn_len    = std::sqrt(wallNormal.x * wallNormal.x +
                                          wallNormal.y * wallNormal.y +
                                          wallNormal.z * wallNormal.z);
        Vec3 const wallNUnit  = (wn_len > 1e-8f) ? wallNormal / wn_len : state.normal;

        auto makeWallVert = [&](uint32_t srcIdx, Vec3 const& wn) -> uint32_t {
            uint32_t const idx = static_cast<uint32_t>(verts.size());
            EditVertex wv      = verts[srcIdx];
            wv.normal          = wn;
            verts.push_back(wv);
            return idx;
        };
        uint32_t const wA  = makeWallVert(A, wallNUnit);
        uint32_t const wnA = makeWallVert(nA, wallNUnit);
        uint32_t const wnB = makeWallVert(nB, wallNUnit);
        uint32_t const wB  = makeWallVert(B, wallNUnit);

        indices.push_back(wA);
        indices.push_back(wnA);
        indices.push_back(wnB);
        indices.push_back(wA);
        indices.push_back(wnB);
        indices.push_back(wB);

        state.wallTopMirrors.emplace_back(wnA, nA);
        state.wallTopMirrors.emplace_back(wnB, nB);
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
        EditVertex copy       = verts[vi];
        verts.push_back(copy);
        state.grabbedVerts.push_back(newIdx);
        state.basePositions.push_back(copy.position);
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
    state.verticesBefore    = m_EditMesh.vertices;  // full snapshot for undo

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

void EditModeSystem::UpdateGrab(float dx, float dy) {
    if (!m_ExtrudeState || !m_Scene->HasComponent<TransformComponent>(m_EditMesh.GetEntity())) {
        return;
    }

    auto& state        = *m_ExtrudeState;
    auto const& tc     = m_Scene->GetComponent<TransformComponent>(m_EditMesh.GetEntity());
    Matx4f const model = m_GlobalTransform * tc.GetMatrix();

    // Project the extrude normal onto the screen to find which 2-D direction it points.
    Vec3 center{0.0f, 0.0f, 0.0f};
    for (Vec3 const& p : state.basePositions) { center = center + p; }
    if (!state.basePositions.empty()) {
        center = center / static_cast<float>(state.basePositions.size());
    }
    Vec2 const s0      = WorldToScreen(center, model);
    Vec2 const s1      = WorldToScreen(center + state.normal * 0.5f, model);
    float const sdx    = s1.x - s0.x;
    float const sdy    = s1.y - s0.y;
    float const sdLen  = std::sqrt(sdx * sdx + sdy * sdy);
    float const invLen = (sdLen > 1e-4f) ? 1.0f / sdLen : 0.0f;
    // Normalised screen-space direction of the extrude normal; fallback to "up" (−Y screen).
    float const ndx = (sdLen > 1e-4f) ? sdx * invLen : 0.0f;
    float const ndy = (sdLen > 1e-4f) ? sdy * invLen : -1.0f;

    // Signed pixel count along the projected normal direction.
    float const proj = dx * ndx + dy * ndy;
    state.offset += proj * 0.005f;

    for (size_t i = 0; i < state.grabbedVerts.size(); ++i) {
        uint32_t const vi = state.grabbedVerts[i];
        if (vi >= m_EditMesh.vertices.size()) { continue; }
        m_EditMesh.vertices[vi].position = state.basePositions[i] + state.normal * state.offset;
    }

    // Keep wall-top verts in sync with their grabbed counterparts.
    for (auto const& [wallVert, grabbedVert] : state.wallTopMirrors) {
        if (wallVert < m_EditMesh.vertices.size() && grabbedVert < m_EditMesh.vertices.size()) {
            m_EditMesh.vertices[wallVert].position = m_EditMesh.vertices[grabbedVert].position;
        }
    }

    FlushToGPU(false);
    RebuildSelectionBuffer();
}

void EditModeSystem::ConfirmGrab() {
    if (!m_ExtrudeState) { return; }
    auto* hist = m_Scene->GetSystem<HistorySystem>();
    if (hist != nullptr && m_EditMesh.IsActive()) {
        hist->Push(std::make_unique<MeshEditCommand>(
            m_Scene, m_EditMesh.GetEntity(), this, m_ExtrudeState->verticesBefore,
            m_ExtrudeState->indicesBefore, m_EditMesh.vertices, m_EditMesh.indices));
    }
    m_ExtrudeState.reset();
    // GPU already reflects the final state from the last UpdateGrab call.
}

void EditModeSystem::SyncFromVertices(std::vector<EditVertex> const& verts,
                                      std::vector<uint32_t> const& inds) {
    m_EditMesh.vertices = verts;
    m_EditMesh.indices  = inds;
    m_EditMesh.ClearSelection();
    m_ExtrudeState.reset();
    RebuildSelectionBuffer();
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
