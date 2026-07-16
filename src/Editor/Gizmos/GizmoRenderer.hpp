// Translation and rotation gizmos for the selected entity; owns drag state and hover detection.
#pragma once
#include "Editor/Gizmos/Gizmo.hpp"
#include "Core/Entity.hpp"
#include "Core/Components/Component.hpp"
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

    // Draw gizmo for the current selection. Call inside the viewport FBO bind, after scene render.
    void Draw();

    // Returns true if the event was consumed by the gizmo (suppresses SelectionSystem).
    bool OnMouseButtonPressed(float viewportX, float viewportY);
    bool OnMouseButtonReleased();
    bool OnMouseMoved(float viewportX, float viewportY);

    void SetViewportSize(uint32_t w, uint32_t h);
    void SetGlobalTransform(const Matx4f& global) { m_GlobalTransform = global; }
    void SetMode(GizmoMode mode) { m_Mode = mode; }
    GizmoMode GetMode() const { return m_Mode; }

private:
    std::shared_ptr<Mesh> BuildArrowMesh();
    std::shared_ptr<Mesh> BuildCenterMesh();
    std::shared_ptr<Mesh> BuildRingMesh();

    GizmoAxis HitTestAxes(float mouseX, float mouseY);
    GizmoAxis HitTestRings(float mouseX, float mouseY);
    Vec3      ScreenToRayDirection(float x, float y) const;
    Vec3      RayAxisClosestPoint(Vec3 rayOrigin, Vec3 rayDir,
                                  Vec3 axisOrigin, Vec3 axisDir) const;
    // Returns false if ray is too parallel to the plane (grazing angle).
    bool      RayPlaneIntersect(Vec3 rayOrigin, Vec3 rayDir,
                                Vec3 planePoint, Vec3 planeNormal,
                                float& outT) const;
    static void RingRefVectors(GizmoAxis axis, Vec3& outU, Vec3& outV);

    Scene&            m_Scene;
    SelectionContext& m_SelCtx;
    const Camera&     m_Camera;

    GizmoMode m_Mode = GizmoMode::Translation;

    std::shared_ptr<Mesh> m_ArrowMesh;
    std::shared_ptr<Mesh> m_CenterMesh;
    std::shared_ptr<Mesh> m_RingMesh;

    bool      m_IsDragging     = false;
    GizmoAxis m_DragAxis       = GizmoAxis::None;
    Vec3      m_DragStartHitPt {0.0f, 0.0f, 0.0f};

    // Rotation drag state
    Vec3 m_RotDragRefPoint {0.0f, 0.0f, 0.0f};
    Vec3 m_RotDragRefU     {1.0f, 0.0f, 0.0f};
    Vec3 m_RotDragRefV     {0.0f, 0.0f, 1.0f};

    GizmoAxis          m_HoveredAxis         = GizmoAxis::None;
    entt::entity       m_DragEntity          = entt::null;
    TransformComponent m_TransformAtDragStart;
    Matx4f             m_GlobalTransform     = Matx4f::identity();

    HistorySystem* m_HistSys = nullptr;

    uint32_t  m_ViewportW = 1470;
    uint32_t  m_ViewportH =  810;
};
