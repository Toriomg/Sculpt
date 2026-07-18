// Manages edit mode lifecycle for one entity at a time; renders vertex/edge overlay.
#pragma once
#include "Core/EditMesh/EditMesh.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"

class Scene;

class EditModeSystem {
public:
    explicit EditModeSystem(Scene* scene) : m_Scene(scene) { }

    // Enter edit mode for the given entity: copies GPU vertex data to CPU-side EditMesh.
    void Enter(entt::entity entity);
    // Exit edit mode: writes CPU vertex data back to GPU and clears EditMesh.
    void Exit();

    [[nodiscard]] bool IsActive() const { return m_EditMesh.IsActive(); }
    [[nodiscard]] entt::entity GetEditedEntity() const { return m_EditMesh.GetEntity(); }
    [[nodiscard]] EditMesh const& GetEditMesh() const { return m_EditMesh; }

    // Render edge-line + vertex-dot overlay for the active entity.
    // Call after Scene::OnUpdate, inside the viewport FBO, after Renderer::BeginScene has run.
    void DrawOverlay(Matx4f const& globalTransform) const;

private:
    Scene* m_Scene;
    EditMesh m_EditMesh;
};
