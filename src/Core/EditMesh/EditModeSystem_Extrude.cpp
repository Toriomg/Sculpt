#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/Commands/MeshEditCommand.hpp"
#include "Core/Systems/HistorySystem.hpp"

#include <cmath>
#include <map>
#include <unordered_map>
#include <vector>

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
            m_ExtrudeState->indicesBefore, m_EditMesh.vertices, m_EditMesh.indices, "Extrude"));
    }
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
