#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/Commands/MeshEditCommand.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Renderer/Camera.hpp"

#include <cmath>
#include <map>
#include <set>
#include <vector>

// ---------------------------------------------------------------------------
// Loop cut
// ---------------------------------------------------------------------------

void EditModeSystem::DoLoopCut(uint32_t primitiveID, float screenX, float screenY) {
    auto& verts   = m_EditMesh.vertices;
    auto& indices = m_EditMesh.indices;

    if (!m_Scene->HasComponent<TransformComponent>(m_EditMesh.GetEntity())) { return; }
    auto const& tc     = m_Scene->GetComponent<TransformComponent>(m_EditMesh.GetEntity());
    Matx4f const model = m_GlobalTransform * tc.GetMatrix();

    // Position quantization — same approach as DoExtrudeFaces for position-based edge identity.
    auto qf     = [](float v) { return static_cast<int32_t>(llroundf(v * 1.0e4f)); };
    using PK    = std::array<int32_t, 3>;
    auto qp     = [&qf](Vec3 const& p) -> PK { return {qf(p.x), qf(p.y), qf(p.z)}; };
    using EK    = std::pair<PK, PK>;
    auto makeEK = [&qp](Vec3 const& pa, Vec3 const& pb) -> EK {
        PK ka = qp(pa), kb = qp(pb);
        return ka < kb ? EK{ka, kb} : EK{kb, ka};
    };

    // Build edge → [face_index, local position of A, B, C within the triangle].
    struct FaceEdge {
        uint32_t fi;
        uint32_t posA, posB, posC;
    };
    std::map<EK, std::vector<FaceEdge>> edgeFaces;
    uint32_t const faceCount = m_EditMesh.FaceCount();
    for (uint32_t fi = 0; fi < faceCount; ++fi) {
        uint32_t const base = fi * 3;
        for (uint32_t k = 0; k < 3; ++k) {
            uint32_t const posA = k;
            uint32_t const posB = (k + 1) % 3;
            uint32_t const posC = (k + 2) % 3;
            uint32_t const ia   = indices[base + posA];
            uint32_t const ib   = indices[base + posB];
            if (ia >= verts.size() || ib >= verts.size()) { continue; }
            edgeFaces[makeEK(verts[ia].position, verts[ib].position)].push_back(
                {fi, posA, posB, posC});
        }
    }

    // Find start edge from click.
    uint32_t const triBase  = primitiveID * 3;
    uint64_t const startKey = FindClosestEdge(triBase, screenX, screenY, model);
    auto const sA           = static_cast<uint32_t>(startKey >> 32U);
    auto const sB           = static_cast<uint32_t>(startKey & 0xFFFFFFFFULL);
    if (sA >= verts.size() || sB >= verts.size()) { return; }
    EK const startEK = makeEK(verts[sA].position, verts[sB].position);

    // Traverse the loop: each step goes from edge E=(A,B) shared by faces F1 and F2
    // to the edge connecting the third vertices of F1 and F2 (the "opposite" diagonal).
    std::vector<EK> loopEdges;
    std::set<EK> visited;
    EK current = startEK;
    while (true) {
        if (visited.count(current) != 0U) { break; }
        visited.insert(current);
        loopEdges.push_back(current);

        auto const it = edgeFaces.find(current);
        if (it == edgeFaces.end() || it->second.size() != 2) { break; }

        uint32_t const c1 = indices[it->second[0].fi * 3 + it->second[0].posC];
        uint32_t const c2 = indices[it->second[1].fi * 3 + it->second[1].posC];
        if (c1 >= verts.size() || c2 >= verts.size()) { break; }

        current = makeEK(verts[c1].position, verts[c2].position);
    }

    if (loopEdges.empty()) { return; }
    std::set<EK> const loopSet(loopEdges.begin(), loopEdges.end());

    // Rebuild the index buffer, splitting each face that contains a loop edge.
    std::vector<uint32_t> newIndices;
    newIndices.reserve(indices.size() * 2);

    for (uint32_t fi = 0; fi < faceCount; ++fi) {
        uint32_t const base = fi * 3;
        uint32_t const v[3] = {indices[base], indices[base + 1], indices[base + 2]};
        if (v[0] >= verts.size() || v[1] >= verts.size() || v[2] >= verts.size()) {
            newIndices.push_back(v[0]);
            newIndices.push_back(v[1]);
            newIndices.push_back(v[2]);
            continue;
        }

        // Check each edge of the face (consecutive pairs in CCW order).
        int cutK = -1;
        for (int k = 0; k < 3; ++k) {
            EK const ek =
                makeEK(verts[v[k]].position, verts[v[static_cast<uint32_t>((k + 1) % 3)]].position);
            if (loopSet.count(ek) != 0U) {
                cutK = k;
                break;
            }
        }

        if (cutK < 0) {
            // No cut on this face — keep as-is.
            newIndices.push_back(v[0]);
            newIndices.push_back(v[1]);
            newIndices.push_back(v[2]);
            continue;
        }

        // posA=(cutK), posB=(cutK+1)%3, posC=(cutK+2)%3 — the split edge and its opposite.
        uint32_t const posA = static_cast<uint32_t>(cutK);
        uint32_t const posB = static_cast<uint32_t>((cutK + 1) % 3);
        uint32_t const posC = static_cast<uint32_t>((cutK + 2) % 3);
        uint32_t const vA = v[posA], vB = v[posB], vC = v[posC];

        // Midpoint normal: interpolate from the two edge endpoints so we never have to
        // guess the winding convention — the stored normals are already correct.
        Vec3 rawN       = verts[vA].normal + verts[vB].normal;
        float const nl  = std::sqrt(rawN.x * rawN.x + rawN.y * rawN.y + rawN.z * rawN.z);
        Vec3 const midN = (nl > 1e-8f) ? rawN / nl : verts[vA].normal;

        // Add midpoint vertex.
        uint32_t const M = static_cast<uint32_t>(verts.size());
        EditVertex mv;
        mv.position = (verts[vA].position + verts[vB].position) / 2.0f;
        mv.normal   = midN;
        mv.texCoord = (verts[vA].texCoord + verts[vB].texCoord) / 2.0f;
        verts.push_back(mv);

        // Two CCW sub-triangles replacing the original:
        // (vA, M, vC) and (M, vB, vC)
        newIndices.push_back(vA);
        newIndices.push_back(M);
        newIndices.push_back(vC);
        newIndices.push_back(M);
        newIndices.push_back(vB);
        newIndices.push_back(vC);
    }

    m_EditMesh.indices        = std::move(newIndices);
    m_EditMesh.selectionDirty = true;
}

void EditModeSystem::LoopCut(uint32_t primitiveID, float screenX, float screenY) {
    if (!m_EditMesh.IsActive() || primitiveID >= m_EditMesh.FaceCount()) { return; }

    auto const vertsBefore = m_EditMesh.vertices;
    auto const indsBefore  = m_EditMesh.indices;

    DoLoopCut(primitiveID, screenX, screenY);

    auto* hist = m_Scene->GetSystem<HistorySystem>();
    if (hist != nullptr) {
        hist->Push(std::make_unique<MeshEditCommand>(m_Scene, m_EditMesh.GetEntity(), this,
                                                     vertsBefore, indsBefore, m_EditMesh.vertices,
                                                     m_EditMesh.indices, "Loop Cut"));
    }

    m_LoopCutMode = false;
    FlushToGPU(true);
    RebuildSelectionBuffer();
}
