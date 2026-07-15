// Translation gizmo: renders 3-axis arrows for the selected entity; owns drag state and hover detection.
#pragma once
#include "Editor/Gizmos/Gizmo.hpp"
#include "Core/Entity.hpp"
#include "Platform/CoreUtils/Math/matx.hpp"
#include <memory>
#include <cstdint>

class Scene;
class SelectionContext;
class Camera;
class Mesh;
class HistorySystem;

class GizmoRenderer {
public:
    GizmoRenderer(Scene& scene, SelectionContext& selCtx, const Camera& camera);
    ~GizmoRenderer();
    GizmoRenderer(const GizmoRenderer&)            = delete;
    GizmoRenderer& operator=(const GizmoRenderer&) = delete;
    GizmoRenderer(GizmoRenderer&&)                 = default;
    GizmoRenderer& operator=(GizmoRenderer&&)      = delete;

    void OnAttach();

    // Draw gizmo arrows for the current selection. Call inside the viewport FBO bind, after scene render.
    void Draw();

    // Returns true if the event was consumed by the gizmo (suppresses SelectionSystem).
    bool OnMouseButtonPressed(float viewportX, float viewportY);
    bool OnMouseButtonReleased();
    bool OnMouseMoved(float viewportX, float viewportY);

    void SetViewportSize(uint32_t w, uint32_t h);

private:
    std::shared_ptr<Mesh> BuildArrowMesh();
    std::shared_ptr<Mesh> BuildCenterMesh();

    GizmoAxis HitTestAxes(float mouseX, float mouseY);
    Vec3      ScreenToRayDirection(float x, float y) const;
    // Returns the point on the axis (through axisOrigin along axisDir) closest to the given ray.
    Vec3      RayAxisClosestPoint(Vec3 rayOrigin, Vec3 rayDir,
                                  Vec3 axisOrigin, Vec3 axisDir) const;

    Scene&            m_Scene;
    SelectionContext& m_SelCtx;
    const Camera&     m_Camera;

    std::shared_ptr<Mesh> m_ArrowMesh;
    std::shared_ptr<Mesh> m_CenterMesh;

    bool      m_IsDragging      = false;
    GizmoAxis m_DragAxis        = GizmoAxis::None;
    Vec3      m_DragStartHitPt  {0.0f, 0.0f, 0.0f};

    GizmoAxis    m_HoveredAxis          = GizmoAxis::None;
    entt::entity m_DragEntity          = entt::null;
    Matx4f       m_TransformAtDragStart = Matx4f::identity();

    HistorySystem* m_HistSys = nullptr;

    uint32_t  m_ViewportW = 1470;
    uint32_t  m_ViewportH =  810;
};
