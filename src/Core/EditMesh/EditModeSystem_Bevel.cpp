#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/Commands/MeshEditCommand.hpp"
#include "Core/Systems/HistorySystem.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_set>
#include <vector>

// ---------------------------------------------------------------------------
// Bevel edges
// ---------------------------------------------------------------------------

void EditModeSystem::DoBevelEdges(BevelState& state) {
    auto& verts          = m_EditMesh.vertices;
    auto& indices        = m_EditMesh.indices;
    auto const& selEdges = m_EditMesh.selectedEdges;
    if (selEdges.empty()) { return; }

    // Snapshot indices before any modification. Redirects modify `indices` live, but
    // per-edge lookups (source vertex ids, edge→face map, face normals) must read the
    // original topology — otherwise the second selected edge sees corruption from the first.
    std::vector<uint32_t> const origIndices = indices;
    uint32_t const origFaceCount            = static_cast<uint32_t>(origIndices.size() / 3);

    auto qf     = [](float v) { return static_cast<int32_t>(llroundf(v * 1.0e4f)); };
    using PK    = std::array<int32_t, 3>;
    auto qp     = [&qf](Vec3 const& p) -> PK { return {qf(p.x), qf(p.y), qf(p.z)}; };
    using EK    = std::pair<PK, PK>;
    auto makeEK = [&qp](Vec3 const& pa, Vec3 const& pb) -> EK {
        PK ka = qp(pa), kb = qp(pb);
        return ka < kb ? EK{ka, kb} : EK{kb, ka};
    };

    auto normalise = [](Vec3 v) -> Vec3 {
        float const l = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        return (l > 1e-8f) ? v / l : Vec3{0.0f, 1.0f, 0.0f};
    };

    struct FaceEdge {
        uint32_t fi;
        uint32_t posA, posB, posC;
    };
    std::map<EK, std::vector<FaceEdge>> edgeFaces;
    for (uint32_t fi = 0; fi < origFaceCount; ++fi) {
        uint32_t const base = fi * 3;
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const posA = k, posB = (k + 1) % 3, posC = (k + 2) % 3;
            uint32_t const ia = origIndices[base + posA];
            uint32_t const ib = origIndices[base + posB];
            if (ia >= verts.size() || ib >= verts.size()) { continue; }
            edgeFaces[makeEK(verts[ia].position, verts[ib].position)].push_back(
                {fi, posA, posB, posC});
        }
    }

    // Vertex-index → faces referencing it (from snapshot). Bounded scan for redirects,
    // avoiding the O(faces × edges) whole-mesh scan the previous implementation used.
    std::vector<std::vector<uint32_t>> vertToFaces(verts.size());
    for (uint32_t fi = 0; fi < origFaceCount; ++fi) {
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const vi = origIndices[fi * 3 + k];
            if (vi < verts.size()) { vertToFaces[vi].push_back(fi); }
        }
    }

    auto origGeoNormal = [&](uint32_t fi) -> Vec3 {
        uint32_t const b = fi * 3;
        Vec3 const p0    = verts[origIndices[b]].position;
        Vec3 const p1    = verts[origIndices[b + 1]].position;
        Vec3 const p2    = verts[origIndices[b + 2]].position;
        return normalise((p1 - p0).crossProduct(p2 - p0));
    };

    // (source vertex index, quantized face normal) → index into state.offsetVerts.
    // Keyed this way, all triangles that share a corner on the same "face group" (coplanar
    // neighbours of the same source vertex) map to a single offset vertex. Two selected
    // edges meeting at that corner accumulate their perp directions into a bisector so
    // both bevel strips connect there without a gap.
    using CornerKey = std::pair<uint32_t, PK>;
    std::map<CornerKey, size_t> cornerToOffset;
    std::unordered_set<uint32_t> affectedFacesSet;

    // Max width: half the shortest selected edge. Keeps offsets from crossing over.
    float minEdgeLen = 1e30f;
    for (uint64_t ek : selEdges) {
        auto const viA = static_cast<uint32_t>(ek >> 32U);
        auto const viB = static_cast<uint32_t>(ek & 0xFFFFFFFFULL);
        if (viA >= verts.size() || viB >= verts.size()) { continue; }
        Vec3 const d  = verts[viB].position - verts[viA].position;
        float const l = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
        if (l > 1e-6f && l < minEdgeLen) { minEdgeLen = l; }
    }
    state.maxWidth = (minEdgeLen < 1e29f) ? (minEdgeLen * 0.5f) : 1.0f;

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
        Vec3 const n1      = origGeoNormal(f1.fi);
        Vec3 const n2      = origGeoNormal(f2.fi);

        auto perpForFace = [&](FaceEdge const& fe, Vec3 const& n) -> Vec3 {
            uint32_t const base = fe.fi * 3;
            Vec3 const pA       = verts[origIndices[base + fe.posA]].position;
            Vec3 const pB       = verts[origIndices[base + fe.posB]].position;
            Vec3 const pC       = verts[origIndices[base + fe.posC]].position;
            Vec3 const e        = normalise(pB - pA);
            Vec3 perp           = normalise(n.crossProduct(e));
            Vec3 const mid      = (pA + pB) * 0.5f;
            Vec3 const toC      = pC - mid;
            if (perp.dotProduct(toC) < 0.0f) { perp = perp * -1.0f; }
            return perp;
        };
        Vec3 const perp1 = perpForFace(f1, n1);
        Vec3 const perp2 = perpForFace(f2, n2);

        // Slot resolution using SNAPSHOT positions — face indices may have been rewritten
        // by an earlier edge in the selection.
        auto slotForPos = [&](FaceEdge const& fe, Vec3 const& refPos) -> uint32_t {
            Vec3 const candPos = verts[origIndices[fe.fi * 3 + fe.posA]].position;
            return (qp(candPos) == qp(refPos)) ? fe.posA : fe.posB;
        };
        uint32_t const f1_slotA = slotForPos(f1, posA);
        uint32_t const f1_slotB = (f1_slotA == f1.posA) ? f1.posB : f1.posA;
        uint32_t const f2_slotA = slotForPos(f2, posA);
        uint32_t const f2_slotB = (f2_slotA == f2.posA) ? f2.posB : f2.posA;

        uint32_t const srcA_f1 = origIndices[f1.fi * 3 + f1_slotA];
        uint32_t const srcB_f1 = origIndices[f1.fi * 3 + f1_slotB];
        uint32_t const srcA_f2 = origIndices[f2.fi * 3 + f2_slotA];
        uint32_t const srcB_f2 = origIndices[f2.fi * 3 + f2_slotB];

        auto getOrCreateOffset = [&](uint32_t srcIdx, Vec3 const& faceN, Vec3 const& basePos,
                                     Vec3 const& dir) -> uint32_t {
            CornerKey const key{srcIdx, qp(faceN)};
            if (auto found = cornerToOffset.find(key); found != cornerToOffset.end()) {
                // Same corner already claimed by an earlier selected edge on the same
                // face group. Blend directions so the shared corner moves along the bisector.
                auto& ov     = state.offsetVerts[found->second];
                ov.direction = normalise(ov.direction + dir);
                return ov.idx;
            }
            uint32_t const newIdx  = static_cast<uint32_t>(verts.size());
            EditVertex const srcVt = verts[srcIdx];
            verts.push_back(srcVt);
            verts.back().position = basePos;
            state.offsetVerts.push_back({newIdx, basePos, dir});
            cornerToOffset[key] = state.offsetVerts.size() - 1;
            return newIdx;
        };

        uint32_t const A1 = getOrCreateOffset(srcA_f1, n1, posA, perp1);
        uint32_t const B1 = getOrCreateOffset(srcB_f1, n1, posB, perp1);
        uint32_t const A2 = getOrCreateOffset(srcA_f2, n2, posA, perp2);
        uint32_t const B2 = getOrCreateOffset(srcB_f2, n2, posB, perp2);

        // Redirect any triangle that (a) originally referenced this specific vertex INDEX
        // and (b) is coplanar with the target face — this covers the two adjacent triangles
        // plus their flat-shaded quad partner(s). `dst` is already the shared offset vertex
        // from getOrCreateOffset, so overwrites from later edges are no-ops instead of
        // corruption.
        auto redirect = [&](uint32_t src, uint32_t dst, Vec3 const& fn) {
            if (src >= vertToFaces.size()) { return; }
            for (uint32_t fi2 : vertToFaces[src]) {
                Vec3 const tfn = origGeoNormal(fi2);
                if (tfn.dotProduct(fn) < 0.99f) { continue; }
                uint32_t const b = fi2 * 3;
                for (uint32_t k = 0; k < 3; ++k) {
                    if (origIndices[b + k] == src) {
                        indices[b + k] = dst;
                        affectedFacesSet.insert(fi2);
                    }
                }
            }
        };
        redirect(srcA_f1, A1, n1);
        redirect(srcB_f1, B1, n1);
        redirect(srcA_f2, A2, n2);
        redirect(srcB_f2, B2, n2);

        // Winding: pick the triangulation whose outward normal aligns with (n1 + n2).
        // (perp2 - perp1) × edgeDir is the strip normal at small width; falls back safely
        // for coplanar faces because |perp2 - perp1| stays substantial (perp1 and perp2 point
        // away from the shared edge into their respective face interiors).
        Vec3 const edgeDirN   = normalise(posB - posA);
        Vec3 const predictedN = (perp2 - perp1).crossProduct(edgeDirN);
        Vec3 const avgFaceN   = n1 + n2;
        float const align     = predictedN.dotProduct(avgFaceN);
        bool const flip       = align < 0.0f;

        uint32_t const stripFace0 = static_cast<uint32_t>(indices.size() / 3);
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
        affectedFacesSet.insert(stripFace0);
        affectedFacesSet.insert(stripFace0 + 1);
    }

    state.affectedFaces.assign(affectedFacesSet.begin(), affectedFacesSet.end());
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

    // Use combined mouse delta so movement in any screen direction affects width.
    // Sign follows the dominant axis (right/up increases width, left/down decreases).
    float const dominant = (std::abs(dx) >= std::abs(dy)) ? dx : -dy;
    state.width          = std::clamp(state.width + dominant * 0.005f, 0.0f, state.maxWidth);

    auto& verts   = m_EditMesh.vertices;
    auto& indices = m_EditMesh.indices;
    for (auto const& ov : state.offsetVerts) {
        if (ov.idx >= verts.size()) { continue; }
        verts[ov.idx].position = ov.basePos + ov.direction * state.width;
    }

    // Smooth-shade the offset verts by area-weighted average of the faces touching them.
    // Without this the bevel strip inherits stale normals from the source vertices.
    std::unordered_set<uint32_t> touched;
    for (auto const& ov : state.offsetVerts) { touched.insert(ov.idx); }
    for (uint32_t vi : touched) {
        if (vi < verts.size()) { verts[vi].normal = Vec3{0.0f, 0.0f, 0.0f}; }
    }
    for (uint32_t fi : state.affectedFaces) {
        uint32_t const b = fi * 3;
        if (b + 2 >= indices.size()) { continue; }
        uint32_t const ia = indices[b];
        uint32_t const ib = indices[b + 1];
        uint32_t const ic = indices[b + 2];
        if (ia >= verts.size() || ib >= verts.size() || ic >= verts.size()) { continue; }
        Vec3 const fn = (verts[ib].position - verts[ia].position)
                            .crossProduct(verts[ic].position - verts[ia].position);
        for (uint32_t vi : {ia, ib, ic}) {
            if (touched.contains(vi)) { verts[vi].normal = verts[vi].normal + fn; }
        }
    }
    for (uint32_t vi : touched) {
        if (vi >= verts.size()) { continue; }
        Vec3 const& n = verts[vi].normal;
        float const l = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        if (l > 1e-8f) { verts[vi].normal = n / l; }
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
    // Original edge keys now reference vertices in the middle of the bevel strip;
    // keeping them would draw the highlight in the wrong place and feed stale ids to
    // the next edit op.
    m_EditMesh.selectedEdges.clear();
    m_EditMesh.selectionDirty = true;
    m_BevelState.reset();
    RebuildSelectionBuffer();
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
