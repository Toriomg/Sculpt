// Translation, rotation, and scale gizmos for the selected entity; owns drag state and hover
// detection.
#pragma once
#include "Core/Components/Component.hpp"
#include "Core/Entity.hpp"
#include "Editor/Gizmos/Gizmo.hpp"
#include <cstdint>
#include <memory>
#include <optional>

class Scene;
class SelectionContext;
class Camera;
class Mesh;
class HistorySystem;

class GizmoRenderer {
public:
    GizmoRenderer(Scene& scene, SelectionContext& selCtx, Camera const& camera);
    ~GizmoRenderer();
    GizmoRenderer(GizmoRenderer const&)            = delete;
    GizmoRenderer& operator=(GizmoRenderer const&) = delete;
    GizmoRenderer(GizmoRenderer&&)                 = default;
    GizmoRenderer& operator=(GizmoRenderer&&)      = delete;

    void OnAttach();

    // Draw gizmo for the current selection. Call inside the viewport FBO bind, after scene render.
    void Draw();

    // Returns true if the event was consumed by the gizmo (suppresses SelectionSystem).
    bool OnMouseButtonPressed(float vx, float vy);
    bool OnMouseButtonReleased();
    bool OnMouseMoved(float vx, float vy);

    struct SnapConfig {
        float translate = 1.0f;   // world units
        float rotate    = 15.0f;  // degrees
        float scale     = 0.1f;   // absolute scale increment
    };

    void SetViewportSize(uint32_t w, uint32_t h);
    void SetGlobalTransform(Matx4f const& global) { m_GlobalTransform = global; }
    void SetMode(GizmoMode mode) { m_Mode = mode; }
    GizmoMode GetMode() const { return m_Mode; }
    void ToggleSpace() {
        m_Space = (m_Space == GizmoSpace::Global) ? GizmoSpace::Local : GizmoSpace::Global;
    }
    void SetSnapConfig(SnapConfig cfg) { m_Snap = cfg; }

private:
    // Returns the world-space direction of axis in the current gizmo space.
    Vec3 AxisDir(GizmoAxis axis, TransformComponent const& tc) const;

    void ApplyTranslationDrag(TransformComponent& tc, Vec3 pos, Vec3 ray, bool snap);
    void ApplyScaleDrag(TransformComponent& tc, Vec3 pos, Vec3 ray, bool snap);
    void ApplyRotationDrag(TransformComponent& tc, Vec3 pos, Vec3 ray, bool snap);

    GizmoAxis HitTestAxes(float mouseX, float mouseY);
    GizmoAxis HitTestRings(float mouseX, float mouseY);
    Vec3 ScreenToRayDirection(float x, float y) const;
    static Vec3 RayAxisClosestPoint(Vec3 P, Vec3 D, Vec3 Q, Vec3 axisDir);
    // Returns false if ray is too parallel to the plane (grazing angle).
    static bool RayPlaneIntersect(Vec3 rayOrigin, Vec3 rayDir, Vec3 planePoint, Vec3 planeNormal,
                                  float& outT);

    // Resolves selection → world position + view-distance scale. Returns nullopt when nothing is
    // selected.
    struct GizmoCtx {
        entt::entity entity;
        Vec3 pos;
        float scale;
    };
    std::optional<GizmoCtx> ActiveGizmoCtx();

    Scene& m_Scene;
    SelectionContext& m_SelCtx;
    Camera const& m_Camera;

    GizmoMode m_Mode   = GizmoMode::Translation;
    GizmoSpace m_Space = GizmoSpace::Global;

    std::shared_ptr<Mesh> m_ArrowMesh;
    std::shared_ptr<Mesh> m_CenterMesh;
    std::shared_ptr<Mesh> m_RingMesh;
    std::shared_ptr<Mesh> m_ConeMesh;
    std::shared_ptr<Mesh> m_ScaleCubeMesh;

    bool m_IsDragging    = false;
    GizmoAxis m_DragAxis = GizmoAxis::None;
    Vec3 m_DragAxisDir{1.0f, 0.0f, 0.0f};  // world-space axis direction at drag start
    Vec3 m_DragStartHitPt{0.0f, 0.0f, 0.0f};
    float m_DragStartDist = 0.f;

    Vec3 m_RotDragRefPoint{0.0f, 0.0f, 0.0f};
    Vec3 m_RotDragRefU{1.0f, 0.0f, 0.0f};
    Vec3 m_RotDragRefV{0.0f, 0.0f, 1.0f};

    GizmoAxis m_HoveredAxis   = GizmoAxis::None;
    entt::entity m_DragEntity = entt::null;
    TransformComponent m_TransformAtDragStart;
    Matx4f m_GlobalTransform = Matx4f::identity();

    SnapConfig m_Snap;
    HistorySystem* m_HistSys = nullptr;

    uint32_t m_ViewportW = 1470;
    uint32_t m_ViewportH = 810;
};
