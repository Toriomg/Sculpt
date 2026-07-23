// Manages edit mode lifecycle for one entity at a time; handles element selection
// (vertex/edge/face) via picking results and renders the selection overlay.
#pragma once
#include "Core/EditMesh/EditMesh.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

class Scene;
class Camera;
class VertexArray;
class VertexBuffer;

class EditModeSystem {
public:
    EditModeSystem(Scene* scene, Camera const& camera);

    // Enter edit mode for the given entity: copies GPU vertex data to CPU-side EditMesh.
    void Enter(entt::entity entity);
    // Exit edit mode: clears EditMesh.
    void Exit();

    [[nodiscard]] bool IsActive() const { return m_EditMesh.IsActive(); }
    [[nodiscard]] entt::entity GetEditedEntity() const { return m_EditMesh.GetEntity(); }
    [[nodiscard]] EditMesh const& GetEditMesh() const { return m_EditMesh; }

    void SetViewportSize(uint32_t w, uint32_t h);
    void SetGlobalTransform(Matx4f const& t) { m_GlobalTransform = t; }

    void SetElementMode(ElementMode mode);
    [[nodiscard]] ElementMode GetElementMode() const { return m_EditMesh.mode; }

    // Called from EditorLayer when the user clicks in edit mode.
    // primitiveID is gl_PrimitiveID from the picking pass (= triangle index).
    void OnMouseClick(uint32_t primitiveID, float screenX, float screenY, bool additive);

    // Extrude selected elements and immediately enter grab mode (E key).
    void Extrude();
    [[nodiscard]] bool IsGrabActive() const { return m_ExtrudeState.has_value(); }
    // Apply screen-space mouse delta to the active grab (call from OnMouseMoved).
    void UpdateGrab(float dx, float dy);
    // Commit the grab: geometry stays, grab state cleared, undo record pushed.
    void ConfirmGrab();
    // Discard the grab: restore pre-extrude geometry and selection.
    void CancelGrab();

    // Inset selected faces (I key). Mouse drag controls thickness; click or Enter confirms.
    void Inset();
    [[nodiscard]] bool IsInsetActive() const { return m_InsetState.has_value(); }
    void UpdateInset(float dx, float dy);
    void ConfirmInset();
    void CancelInset();

    // Overwrite CPU mesh state without re-reading from GPU; called by undo/redo.
    void SyncFromVertices(std::vector<EditVertex> const& verts, std::vector<uint32_t> const& inds);

    // Render edge-line + vertex-dot overlay, plus selection highlight.
    // Call after Scene::OnUpdate, inside the viewport FBO, after Renderer::BeginScene has run.
    void DrawOverlay(Matx4f const& globalTransform);

private:
    struct InsetState {
        std::vector<EditVertex> verticesBefore;
        std::vector<uint32_t> indicesBefore;
        std::unordered_set<uint32_t> facesBefore;
        // Per inner-vertex: original face center + inset direction (centroid → original pos).
        struct InnerVert {
            uint32_t idx   = 0;     // index in m_EditMesh.vertices
            Vec3 center    = {};    // face centroid (fixed reference point)
            Vec3 direction = {};    // unit vector from center toward original vertex position
            float baseLen  = 0.0f;  // distance from center to original vertex (max inset radius)
        };
        std::vector<InnerVert> innerVerts;
        float thickness = 0.0f;
    };

    struct ExtrudeState {
        size_t vertexCountBefore{};
        std::vector<uint32_t> indicesBefore;
        std::unordered_set<uint32_t> facesBefore;
        std::unordered_set<uint32_t> vertsBefore;
        std::unordered_set<uint64_t> edgesBefore;
        ElementMode modeBefore{ElementMode::Vertex};
        std::vector<uint32_t> grabbedVerts;      // new vertex indices to translate during drag
        std::vector<Vec3> basePositions;         // grabbed vert positions at extrude time
        Vec3 normal{0.0f, 1.0f, 0.0f};           // extrude direction
        float offset{0.0f};                      // accumulated drag distance
        std::vector<EditVertex> verticesBefore;  // full vertex snapshot before extrude (for undo)
        // (wall-top vert, grabbed vert it mirrors): synced in UpdateGrab for correct wall normals
        std::vector<std::pair<uint32_t, uint32_t>> wallTopMirrors;
    };

    void DoExtrudeFaces(ExtrudeState& state);
    void DoExtrudeEdges(ExtrudeState& state);
    void DoExtrudeVerts(ExtrudeState& state);
    void DoInsetFaces(InsetState& state);
    // Convert EditMesh CPU data → GPU. fullRebuild=true when index count changed.
    void FlushToGPU(bool fullRebuild);

    [[nodiscard]] Vec2 WorldToScreen(Vec3 localPos, Matx4f const& modelMatrix) const;
    [[nodiscard]] uint32_t FindClosestVertex(uint32_t triBase, float sx, float sy,
                                             Matx4f const& model) const;
    [[nodiscard]] uint64_t FindClosestEdge(uint32_t triBase, float sx, float sy,
                                           Matx4f const& model) const;
    void RebuildSelectionBuffer();

    Scene* m_Scene;
    Camera const& m_Camera;
    EditMesh m_EditMesh;
    Matx4f m_GlobalTransform = Matx4f::identity();
    uint32_t m_ViewportW     = 1470;
    uint32_t m_ViewportH     = 810;

    std::optional<ExtrudeState> m_ExtrudeState;
    std::optional<InsetState> m_InsetState;

    // Selection highlight VAO/VBO (position-only, rebuilt when selection changes).
    std::shared_ptr<VertexArray> m_SelectionVAO;
    std::shared_ptr<VertexBuffer> m_SelectionVBO;
    uint32_t m_SelectionDrawCount = 0;
    // GL_POINTS=0x0000, GL_LINES=0x0001, GL_TRIANGLES=0x0004 — stored as uint32_t to avoid
    // including GL headers in the header.
    uint32_t m_SelectionPrimitive = 0;
};
