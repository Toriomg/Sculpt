#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/Commands/MeshEditCommand.hpp"
#include "Core/Systems/HistorySystem.hpp"

#include <cmath>
#include <map>
#include <vector>

// ---------------------------------------------------------------------------
// Bevel edges
// ---------------------------------------------------------------------------

void EditModeSystem::DoBevelEdges(BevelState& state) {
    auto& verts          = m_EditMesh.vertices;
    auto& indices        = m_EditMesh.indices;
    auto const& selEdges = m_EditMesh.selectedEdges;
    if (selEdges.empty()) { return; }

    // Position quantization helpers (same pattern as loop cut / extrude).
    auto qf     = [](float v) { return static_cast<int32_t>(llroundf(v * 1.0e4f)); };
    using PK    = std::array<int32_t, 3>;
    auto qp     = [&qf](Vec3 const& p) -> PK { return {qf(p.x), qf(p.y), qf(p.z)}; };
    using EK    = std::pair<PK, PK>;
    auto makeEK = [&qp](Vec3 const& pa, Vec3 const& pb) -> EK {
        PK ka = qp(pa), kb = qp(pb);
        return ka < kb ? EK{ka, kb} : EK{kb, ka};
    };

    // Build position-based edge → face adjacency.
    struct FaceEdge {
        uint32_t fi;
        uint32_t posA, posB, posC;
    };
    std::map<EK, std::vector<FaceEdge>> edgeFaces;
    uint32_t const faceCount = m_EditMesh.FaceCount();
    for (uint32_t fi = 0; fi < faceCount; ++fi) {
        uint32_t const base = fi * 3;
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const posA = k, posB = (k + 1) % 3, posC = (k + 2) % 3;
            uint32_t const ia = indices[base + posA];
            uint32_t const ib = indices[base + posB];
            if (ia >= verts.size() || ib >= verts.size()) { continue; }
            edgeFaces[makeEK(verts[ia].position, verts[ib].position)].push_back(
                {fi, posA, posB, posC});
        }
    }

    // Helper: normalise a Vec3.
    auto normalise = [](Vec3 v) -> Vec3 {
        float const l = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        return (l > 1e-8f) ? v / l : Vec3{0.0f, 1.0f, 0.0f};
    };

    for (uint64_t ek : selEdges) {
        auto const viA = static_cast<uint32_t>(ek >> 32U);
        auto const viB = static_cast<uint32_t>(ek & 0xFFFFFFFFULL);
        if (viA >= verts.size() || viB >= verts.size()) { continue; }

        Vec3 const posA = verts[viA].position;
        Vec3 const posB = verts[viB].position;
        EK const geoEK  = makeEK(posA, posB);

        auto const it = edgeFaces.find(geoEK);
        if (it == edgeFaces.end() || it->second.size() != 2) { continue; }

        FaceEdge const& f1 = it->second[0];
        FaceEdge const& f2 = it->second[1];

        // Geometric face normal (p1-p0 × p2-p0, CCW convention) — accurate for any shading mode.
        auto geoNormal = [&](uint32_t fi) -> Vec3 {
            uint32_t const b = fi * 3;
            Vec3 const p0    = verts[indices[b]].position;
            Vec3 const p1    = verts[indices[b + 1]].position;
            Vec3 const p2    = verts[indices[b + 2]].position;
            return normalise((p1 - p0).crossProduct(p2 - p0));
        };

        // For each face, compute the perp direction (from the shared edge into the face).
        auto perpForFace = [&](FaceEdge const& fe) -> Vec3 {
            uint32_t const base = fe.fi * 3;
            Vec3 const pA       = verts[indices[base + fe.posA]].position;
            Vec3 const pB       = verts[indices[base + fe.posB]].position;
            Vec3 const pC       = verts[indices[base + fe.posC]].position;
            Vec3 const n        = geoNormal(fe.fi);
            Vec3 const e        = normalise(pB - pA);
            Vec3 perp           = normalise(n.crossProduct(e));
            Vec3 const mid      = (pA + pB) * 0.5f;
            Vec3 const toC      = pC - mid;
            if ((perp.x * toC.x + perp.y * toC.y + perp.z * toC.z) < 0.0f) { perp = perp * -1.0f; }
            return perp;
        };

        Vec3 const perp1 = perpForFace(f1);
        Vec3 const perp2 = perpForFace(f2);

        // Determine which local position in each face corresponds to posA (vertex A) vs posB (B).
        // The face may store the edge as A→B or B→A depending on winding.
        auto faceVertIdx = [&](FaceEdge const& fe, Vec3 const& refPos, uint32_t candidate) {
            uint32_t const other = (candidate == fe.posA) ? fe.posB : fe.posA;
            Vec3 const candPos   = verts[indices[fe.fi * 3 + candidate]].position;
            return (qp(candPos) == qp(refPos)) ? candidate : other;
        };
        uint32_t const f1_posForA = faceVertIdx(f1, posA, f1.posA);
        uint32_t const f1_posForB = (f1_posForA == f1.posA) ? f1.posB : f1.posA;
        uint32_t const f2_posForA = faceVertIdx(f2, posA, f2.posA);
        uint32_t const f2_posForB = (f2_posForA == f2.posA) ? f2.posB : f2.posA;

        // Create 4 offset vertices (at width=0 they coincide with A and B).
        // Copy src vertex before push_back to avoid UB if the vector reallocates.
        auto addOffsetVert = [&](uint32_t srcIdx, Vec3 const& basePos,
                                 Vec3 const& dir) -> uint32_t {
            uint32_t const newIdx  = static_cast<uint32_t>(verts.size());
            EditVertex const srcVt = verts[srcIdx];
            verts.push_back(srcVt);
            verts.back().position = basePos;
            state.offsetVerts.push_back({newIdx, basePos, dir});
            return newIdx;
        };

        uint32_t const srcA_f1 = indices[f1.fi * 3 + f1_posForA];
        uint32_t const srcB_f1 = indices[f1.fi * 3 + f1_posForB];
        uint32_t const srcA_f2 = indices[f2.fi * 3 + f2_posForA];
        uint32_t const srcB_f2 = indices[f2.fi * 3 + f2_posForB];

        uint32_t const A1 = addOffsetVert(srcA_f1, posA, perp1);
        uint32_t const B1 = addOffsetVert(srcB_f1, posB, perp1);
        uint32_t const A2 = addOffsetVert(srcA_f2, posA, perp2);
        uint32_t const B2 = addOffsetVert(srcB_f2, posB, perp2);

        // Redirect ALL triangles in the same geometric face group to the offset vertices.
        // A flat-shaded quad has 2 triangles sharing the same vertex indices; updating only
        // f1.fi leaves the partner triangle at the original position, creating a crack.
        // ponytail: normal-based grouping; smooth-mesh vertex corners need half-edge topology.
        Vec3 const n1            = geoNormal(f1.fi);
        Vec3 const n2            = geoNormal(f2.fi);
        auto redirectInFaceGroup = [&](uint32_t src, uint32_t dst, Vec3 const& fn) {
            for (uint32_t fi2 = 0; fi2 < faceCount; ++fi2) {
                Vec3 const tfn = geoNormal(fi2);
                float const d  = tfn.x * fn.x + tfn.y * fn.y + tfn.z * fn.z;
                if (d < 0.99f) { continue; }
                uint32_t const b = fi2 * 3;
                for (uint32_t k = 0; k < 3; ++k) {
                    if (indices[b + k] == src) { indices[b + k] = dst; }
                }
            }
        };
        redirectInFaceGroup(srcA_f1, A1, n1);
        redirectInFaceGroup(srcB_f1, B1, n1);
        redirectInFaceGroup(srcA_f2, A2, n2);
        redirectInFaceGroup(srcB_f2, B2, n2);

        // Bevel strip: quad A1-B1-B2-A2 as two CCW triangles.
        // The predicted strip normal is cross(perp2-perp1, edgeDir); it must align with the
        // average face normal. If it doesn't, flip the winding.
        Vec3 const edgeDirN   = normalise(posB - posA);
        Vec3 const predictedN = (perp2 - perp1).crossProduct(edgeDirN);
        Vec3 const avgFaceN   = geoNormal(f1.fi) + geoNormal(f2.fi);
        bool const flip =
            (predictedN.x * avgFaceN.x + predictedN.y * avgFaceN.y + predictedN.z * avgFaceN.z) <
            0.0f;
        if (!flip) {
            indices.push_back(A1);
            indices.push_back(B2);
            indices.push_back(B1);
            indices.push_back(A1);
            indices.push_back(A2);
            indices.push_back(B2);
        } else {
            indices.push_back(A1);
            indices.push_back(B1);
            indices.push_back(B2);
            indices.push_back(A1);
            indices.push_back(B2);
            indices.push_back(A2);
        }
    }

    m_EditMesh.selectionDirty = true;
}

void EditModeSystem::Bevel() {
    if (!m_EditMesh.IsActive() || m_BevelState.has_value()) { return; }
    if (m_EditMesh.mode != ElementMode::Edge || m_EditMesh.selectedEdges.empty()) { return; }

    BevelState state;
    state.verticesBefore = m_EditMesh.vertices;
    state.indicesBefore  = m_EditMesh.indices;
    state.edgesBefore    = m_EditMesh.selectedEdges;

    DoBevelEdges(state);

    m_BevelState = std::move(state);
    FlushToGPU(true);
    RebuildSelectionBuffer();
}

void EditModeSystem::UpdateBevel(float dx, float dy) {
    if (!m_BevelState) { return; }
    auto& state = *m_BevelState;
    state.width = std::max(0.0f, state.width + dx * 0.005f);

    for (auto const& ov : state.offsetVerts) {
        if (ov.idx >= m_EditMesh.vertices.size()) { continue; }
        m_EditMesh.vertices[ov.idx].position = ov.basePos + ov.direction * state.width;
    }

    FlushToGPU(false);
    RebuildSelectionBuffer();
}

void EditModeSystem::ConfirmBevel() {
    if (!m_BevelState) { return; }
    auto* hist = m_Scene->GetSystem<HistorySystem>();
    if (hist != nullptr && m_EditMesh.IsActive()) {
        hist->Push(std::make_unique<MeshEditCommand>(
            m_Scene, m_EditMesh.GetEntity(), this, m_BevelState->verticesBefore,
            m_BevelState->indicesBefore, m_EditMesh.vertices, m_EditMesh.indices, "Bevel"));
    }
    m_BevelState.reset();
}

void EditModeSystem::CancelBevel() {
    if (!m_BevelState) { return; }
    m_EditMesh.vertices       = m_BevelState->verticesBefore;
    m_EditMesh.indices        = m_BevelState->indicesBefore;
    m_EditMesh.selectedEdges  = m_BevelState->edgesBefore;
    m_EditMesh.selectionDirty = true;
    m_BevelState.reset();
    FlushToGPU(true);
    RebuildSelectionBuffer();
}
