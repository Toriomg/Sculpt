// Manages edit mode lifecycle for one entity at a time; handles element selection
// (vertex/edge/face) via picking results and renders the selection overlay.
#pragma once
#include "Core/EditMesh/EditMesh.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include <cstdint>
#include <memory>

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

    // Render edge-line + vertex-dot overlay, plus selection highlight.
    // Call after Scene::OnUpdate, inside the viewport FBO, after Renderer::BeginScene has run.
    void DrawOverlay(Matx4f const& globalTransform);

private:
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

    // Selection highlight VAO/VBO (position-only, rebuilt when selection changes).
    std::shared_ptr<VertexArray> m_SelectionVAO;
    std::shared_ptr<VertexBuffer> m_SelectionVBO;
    uint32_t m_SelectionDrawCount = 0;
    // GL_POINTS=0x0000, GL_LINES=0x0001, GL_TRIANGLES=0x0004 — stored as uint32_t to avoid
    // including GL headers in the header.
    uint32_t m_SelectionPrimitive = 0;
};
