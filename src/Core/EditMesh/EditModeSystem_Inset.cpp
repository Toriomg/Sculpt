#include "Core/EditMesh/EditModeSystem.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/Commands/MeshEditCommand.hpp"
#include "Core/Systems/HistorySystem.hpp"

#include <cmath>
#include <vector>

// ---------------------------------------------------------------------------
// Inset faces
// ---------------------------------------------------------------------------

void EditModeSystem::DoInsetFaces(InsetState& state) {
    auto& verts          = m_EditMesh.vertices;
    auto& indices        = m_EditMesh.indices;
    auto const& selFaces = m_EditMesh.selectedFaces;
    if (selFaces.empty()) { return; }

    for (uint32_t fi : selFaces) {
        uint32_t const base = fi * 3;
        if (base + 2 >= indices.size()) { continue; }

        uint32_t const i0 = indices[base + 0];
        uint32_t const i1 = indices[base + 1];
        uint32_t const i2 = indices[base + 2];
        if (i0 >= verts.size() || i1 >= verts.size() || i2 >= verts.size()) { continue; }

        Vec3 const p0       = verts[i0].position;
        Vec3 const p1       = verts[i1].position;
        Vec3 const p2       = verts[i2].position;
        Vec3 const centroid = (p0 + p1 + p2) / 3.0f;

        // Face normal — shared by all border quads (coplanar geometry).
        Vec3 const e1  = p1 - p0;
        Vec3 const e2  = p2 - p0;
        Vec3 faceN     = e1.crossProduct(e2);
        float const nl = std::sqrt(faceN.x * faceN.x + faceN.y * faceN.y + faceN.z * faceN.z);
        faceN          = (nl > 1e-8f) ? faceN / nl : Vec3{0.0f, 1.0f, 0.0f};

        // Create three inner vertices (start at original positions; UpdateInset will move them).
        auto addInner = [&](uint32_t srcIdx, Vec3 const& origPos) -> uint32_t {
            uint32_t const newIdx = static_cast<uint32_t>(verts.size());
            EditVertex iv         = verts[srcIdx];
            iv.normal             = faceN;
            verts.push_back(iv);

            Vec3 const dir  = origPos - centroid;
            float const len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
            InsetState::InnerVert iv2;
            iv2.idx       = newIdx;
            iv2.center    = centroid;
            iv2.direction = (len > 1e-8f) ? dir / len : Vec3{0.0f, 0.0f, 0.0f};
            iv2.baseLen   = len;
            state.innerVerts.push_back(iv2);
            return newIdx;
        };

        uint32_t const ii0 = addInner(i0, p0);
        uint32_t const ii1 = addInner(i1, p1);
        uint32_t const ii2 = addInner(i2, p2);

        // Remap the original face to the inner vertices.
        indices[base + 0] = ii0;
        indices[base + 1] = ii1;
        indices[base + 2] = ii2;

        // Also update normals on the original boundary verts to face normal.
        verts[i0].normal = faceN;
        verts[i1].normal = faceN;
        verts[i2].normal = faceN;

        // Three border quads (as 2 CCW triangles each):
        // Edge i0→i1: (i0, i1, ii1, ii0) → (i0,i1,ii1), (i0,ii1,ii0)
        // Edge i1→i2: (i1, i2, ii2, ii1) → (i1,i2,ii2), (i1,ii2,ii1)
        // Edge i2→i0: (i2, i0, ii0, ii2) → (i2,i0,ii0), (i2,ii0,ii2)
        indices.insert(indices.end(), {i0, i1, ii1, i0, ii1, ii0, i1, i2, ii2, i1, ii2, ii1, i2, i0,
                                       ii0, i2, ii0, ii2});
    }

    // Selection: keep the face selection pointing at the inset inner faces.
    m_EditMesh.selectionDirty = true;
}

void EditModeSystem::Inset() {
    if (!m_EditMesh.IsActive() || m_InsetState.has_value()) { return; }
    if (m_EditMesh.mode != ElementMode::Face || m_EditMesh.selectedFaces.empty()) { return; }

    InsetState state;
    state.verticesBefore = m_EditMesh.vertices;
    state.indicesBefore  = m_EditMesh.indices;
    state.facesBefore    = m_EditMesh.selectedFaces;

    DoInsetFaces(state);

    m_InsetState = std::move(state);
    FlushToGPU(true);
    RebuildSelectionBuffer();
}

void EditModeSystem::UpdateInset(float dx, float dy) {
    if (!m_InsetState) { return; }

    auto& state = *m_InsetState;
    // Rightward drag increases thickness; clamp to [0, 1].
    state.thickness = std::clamp(state.thickness + dx * 0.003f, 0.0f, 0.99f);

    for (auto const& iv : state.innerVerts) {
        if (iv.idx >= m_EditMesh.vertices.size()) { continue; }
        m_EditMesh.vertices[iv.idx].position =
            iv.center + iv.direction * (iv.baseLen * (1.0f - state.thickness));
    }

    FlushToGPU(false);
    RebuildSelectionBuffer();
}

void EditModeSystem::ConfirmInset() {
    if (!m_InsetState) { return; }
    auto* hist = m_Scene->GetSystem<HistorySystem>();
    if (hist != nullptr && m_EditMesh.IsActive()) {
        hist->Push(std::make_unique<MeshEditCommand>(
            m_Scene, m_EditMesh.GetEntity(), this, m_InsetState->verticesBefore,
            m_InsetState->indicesBefore, m_EditMesh.vertices, m_EditMesh.indices, "Inset Faces"));
    }
    m_InsetState.reset();
}

void EditModeSystem::CancelInset() {
    if (!m_InsetState) { return; }
    m_EditMesh.vertices       = m_InsetState->verticesBefore;
    m_EditMesh.indices        = m_InsetState->indicesBefore;
    m_EditMesh.selectedFaces  = m_InsetState->facesBefore;
    m_EditMesh.selectionDirty = true;
    m_InsetState.reset();
    FlushToGPU(true);
    RebuildSelectionBuffer();
}
