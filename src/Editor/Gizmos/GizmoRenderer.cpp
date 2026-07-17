#include "Editor/Gizmos/GizmoRenderer.hpp"

#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/Commands/MultiTransformCommand.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Platform/Graphics/RenderCommand.hpp"
#include "Platform/System/Input/Input.hpp"
#include "Platform/System/Input/KeyCodes.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Renderer.hpp"
#include <algorithm>
#include <array>
#include <cmath>

// ---- file-scope helpers -----------------------------------------------------

namespace {

    Vec3 axisToVec3(GizmoAxis a) {
        switch (a) {
            case GizmoAxis::X: return {1.f, 0.f, 0.f};
            case GizmoAxis::Y: return {0.f, 1.f, 0.f};
            case GizmoAxis::Z: return {0.f, 0.f, 1.f};
            default:           return {0.f, 0.f, 0.f};
        }
    }

    // Returns a pointer-to-member for the Vec3 component matching the axis.
    // Lets callers write `vec.*field` instead of an if-else chain per axis.
    float Vec3::* axisToField(GizmoAxis a) {
        switch (a) {
            case GizmoAxis::X: return &Vec3::x;
            case GizmoAxis::Y: return &Vec3::y;
            case GizmoAxis::Z: return &Vec3::z;
            default:           return nullptr;
        }
    }

    // Calls fn(currentTC, startTC) for every entity in the snapshot list.
    // Defined here (not in the header) so Scene::GetComponent is fully visible.
    template <typename Fn>
    void forEachSnapshot(Scene& scene,
                         std::vector<std::pair<entt::entity, TransformComponent>> const& snapshots,
                         Fn fn) {
        for (auto const& [entity, startTC] : snapshots) {
            fn(scene.GetComponent<TransformComponent>(entity), startTC);
        }
    }

    // Extracts ZYX Euler angles in degrees from a pure rotation matrix R = Rz*Ry*Rx.
    // Handles gimbal lock (y = ±90°) by setting z = 0 and solving for x.
    Vec3 matToEulerDegrees(Matx4f const& r) {
        constexpr float kToDeg = 180.f / PI_F;
        float const sy         = std::clamp(-r.m[2][0], -1.f, 1.f);
        float const y          = std::asin(sy) * kToDeg;
        float const cy         = std::sqrt(1.f - sy * sy);
        float x{}, z{};
        if (cy > 1e-4f) {
            x = std::atan2(r.m[2][1], r.m[2][2]) * kToDeg;
            z = std::atan2(r.m[1][0], r.m[0][0]) * kToDeg;
        } else if (sy > 0.f) {
            x = std::atan2(r.m[0][1], r.m[0][2]) * kToDeg;
        } else {
            x = std::atan2(-r.m[0][1], -r.m[0][2]) * kToDeg;
        }
        return {x, y, z};
    }

    float segDist2D(float px, float py, float ax, float ay, float bx, float by) {
        float const abx = bx - ax, aby = by - ay, apx = px - ax, apy = py - ay;
        float const ab2 = abx * abx + aby * aby;
        float const t  = (ab2 > 1e-12f) ? std::clamp((apx * abx + apy * aby) / ab2, 0.f, 1.f) : 0.f;
        float const dx = px - (ax + t * abx), dy = py - (ay + t * aby);
        return std::sqrt(dx * dx + dy * dy);
    }

    Vec2 worldToScreen(Vec3 const& w, Matx4f const& VP, float W, float H) {
        Vec4 const clip = VP * Vec4(w.x, w.y, w.z, 1.f);
        if (std::abs(clip.w) < 1e-6f) { return {-9999.f, -9999.f}; }
        float const nx = clip.x / clip.w, ny = clip.y / clip.w;
        return {(nx * 0.5f + 0.5f) * W, (1.f - (ny * 0.5f + 0.5f)) * H};
    }

    void ringRefVectors(GizmoAxis axis, Vec3& outU, Vec3& outV) {
        switch (axis) {
            case GizmoAxis::X:
                outU = {0, 1, 0};
                outV = {0, 0, 1};
                break;
            case GizmoAxis::Y:
                outU = {1, 0, 0};
                outV = {0, 0, 1};
                break;
            case GizmoAxis::Z:
                outU = {1, 0, 0};
                outV = {0, 1, 0};
                break;
            default:
                outU = {1, 0, 0};
                outV = {0, 0, 1};
                break;
        }
    }

}  // namespace

// ---- GizmoRenderer ----------------------------------------------------------

GizmoRenderer::GizmoRenderer(Scene& scene, SelectionContext& selCtx, Camera const& camera)
    : m_Scene(scene), m_SelCtx(selCtx), m_Camera(camera) { }

GizmoRenderer::~GizmoRenderer() = default;

void GizmoRenderer::OnAttach() {
    m_ArrowMesh = Mesh::CreateArrow();
    // Thin torus in XZ plane (Y = hole axis), unit major radius; re-oriented per ring at draw
    // time.
    m_RingMesh      = Mesh::CreateTorus(1.0f, 0.035f, 64, 10);
    m_CenterMesh    = Mesh::CreateSphere(1.0f, 8, 8);
    m_ConeMesh      = Mesh::CreateCone();
    m_ScaleCubeMesh = Mesh::CreateCube(1.0f);
    m_HistSys       = m_Scene.GetSystem<HistorySystem>();
}

void GizmoRenderer::SetViewportSize(uint32_t w, uint32_t h) {
    m_ViewportW = w;
    m_ViewportH = h;
}

Vec3 GizmoRenderer::AxisDir(GizmoAxis axis, TransformComponent const& tc) const {
    Vec3 const w = axisToVec3(axis);
    if (m_Space == GizmoSpace::Global) { return w; }
    Vec4 const v = Matx4f::rotation(tc.EulerDegrees.x, tc.EulerDegrees.y, tc.EulerDegrees.z) *
                   Vec4(w.x, w.y, w.z, 0.f);
    return {v.x, v.y, v.z};
}

// ---- context ----------------------------------------------------------------

std::optional<GizmoRenderer::GizmoCtx> GizmoRenderer::ActiveGizmoCtx() {
    auto const& sel = m_SelCtx.GetSelectedEntities();
    if (sel.empty()) { return std::nullopt; }

    entt::entity const active = m_SelCtx.GetActiveEntity();
    if (!m_Scene.HasComponent<TransformComponent>(active)) { return std::nullopt; }

    Vec3 pivotPos;
    if (m_PivotMode == PivotMode::MedianPoint) {
        Vec3 sum{};
        int count = 0;
        for (entt::entity e : sel) {
            if (m_Scene.HasComponent<TransformComponent>(e)) {
                sum += m_Scene.GetComponent<TransformComponent>(e).Translation;
                count++;
            }
        }
        if (count == 0) { return std::nullopt; }
        pivotPos = m_GlobalTransform.transformPoint(sum / static_cast<float>(count));
    } else {
        // IndividualOrigins and ActiveElement both display the gizmo at the active entity.
        pivotPos = m_GlobalTransform.transformPoint(
            m_Scene.GetComponent<TransformComponent>(active).Translation);
    }

    float const scale = (pivotPos - m_Camera.GetPosition()).length() * 0.15f;
    return GizmoCtx{active, pivotPos, scale};
}

// ---- rendering --------------------------------------------------------------

void GizmoRenderer::Draw() {
    auto ctx = ActiveGizmoCtx();
    if (!ctx) { return; }

    auto const& tc = m_Scene.GetComponent<TransformComponent>(ctx->entity);
    Matx4f const entRot =
        (m_Space == GizmoSpace::Local)
            ? Matx4f::rotation(tc.EulerDegrees.x, tc.EulerDegrees.y, tc.EulerDegrees.z)
            : Matx4f::identity();

    auto colorFor = [&](GizmoAxis axis, Vec4 base) -> Vec4 {
        if (m_HoveredAxis != axis && m_DragAxis != axis) { return base; }
        return {std::min(base.x * 1.6f, 1.f), std::min(base.y * 1.6f, 1.f),
                std::min(base.z * 1.6f, 1.f), 1.f};
    };

    RenderCommand::SetDepthFunc(DepthFunc::Always);

    if (m_Mode == GizmoMode::Translation) {
        Renderer::SubmitFlat(
            m_ArrowMesh, colorFor(GizmoAxis::X, {1.f, 0.2f, 0.2f, 1.f}),
            Matx4f::translation(ctx->pos) * entRot * Matx4f::scalingScalar(ctx->scale));
        Renderer::SubmitFlat(m_ArrowMesh, colorFor(GizmoAxis::Y, {0.2f, 1.f, 0.2f, 1.f}),
                             Matx4f::translation(ctx->pos) *
                                 entRot *
                                 Matx4f::rotationZ(90.f) *
                                 Matx4f::scalingScalar(ctx->scale));
        Renderer::SubmitFlat(m_ArrowMesh, colorFor(GizmoAxis::Z, {0.2f, 0.2f, 1.f, 1.f}),
                             Matx4f::translation(ctx->pos) *
                                 entRot *
                                 Matx4f::rotationY(-90.f) *
                                 Matx4f::scalingScalar(ctx->scale));
    } else if (m_Mode == GizmoMode::Scale) {
        struct AxisDef {
            GizmoAxis axis = GizmoAxis::None;
            Vec4 color;
            Matx4f rot = Matx4f::identity();
        };
        std::array<AxisDef, 3> const scaleAxes = {
          AxisDef{GizmoAxis::X, {1.f, 0.2f, 0.2f, 1.f},       Matx4f::identity()},
          AxisDef{GizmoAxis::Y, {0.2f, 1.f, 0.2f, 1.f},  Matx4f::rotationZ(90.f)},
          AxisDef{GizmoAxis::Z, {0.2f, 0.2f, 1.f, 1.f}, Matx4f::rotationY(-90.f)},
        };
        constexpr float kCubeSize = 0.12f;
        for (auto const& def : scaleAxes) {
            Vec4 const col     = colorFor(def.axis, def.color);
            Matx4f const shaft = Matx4f::translation(ctx->pos) *
                                 entRot *
                                 def.rot *
                                 Matx4f::scalingScalar(ctx->scale);
            Renderer::SubmitFlat(m_ArrowMesh, col, shaft);
            Vec3 const tipPos = ctx->pos + AxisDir(def.axis, tc) * ctx->scale;
            Renderer::SubmitFlat(
                m_ScaleCubeMesh, col,
                Matx4f::translation(tipPos) * Matx4f::scalingScalar(ctx->scale * kCubeSize));
        }
    } else {
        // Base torus lies in XZ plane; each ring re-oriented via a rotation before the uniform
        // scale. Two cones per ring mark the tangent at ±(1,0,0) in ring-local space.
        constexpr float coneR = 0.12f, coneH = 0.28f;
        struct RingDef {
            GizmoAxis axis{GizmoAxis::None};
            Vec4 color;
            Matx4f rot;
        };
        std::array<RingDef, 3> const rings = {
          RingDef{GizmoAxis::X, {1.f, 0.2f, 0.2f, 1.f}, Matx4f::rotationZ(90.f)},
          RingDef{GizmoAxis::Y, {0.2f, 1.f, 0.2f, 1.f},      Matx4f::identity()},
          RingDef{GizmoAxis::Z, {0.2f, 0.2f, 1.f, 1.f}, Matx4f::rotationX(90.f)},
        };
        Matx4f const cs = Matx4f::scaling(Vec3{coneR, coneR, coneH});
        for (auto const& ring : rings) {
            Vec4 const col    = colorFor(ring.axis, ring.color);
            Matx4f const base = Matx4f::translation(ctx->pos) *
                                entRot *
                                ring.rot *
                                Matx4f::scalingScalar(ctx->scale);
            Renderer::SubmitFlat(m_RingMesh, col, base);
            Renderer::SubmitFlat(m_ConeMesh, col, base * Matx4f::translation(Vec3{1, 0, 0}) * cs);
            Renderer::SubmitFlat(
                m_ConeMesh, col,
                base * Matx4f::translation(Vec3{-1, 0, 0}) * Matx4f::rotationY(180.f) * cs);
        }
    }

    Renderer::SubmitFlat(m_CenterMesh, {0.9f, 0.9f, 0.9f, 1.f},
                         Matx4f::translation(ctx->pos) * Matx4f::scalingScalar(ctx->scale * 0.08f));

    RenderCommand::SetDepthFunc(DepthFunc::Less);
}

// ---- math helpers -----------------------------------------------------------

Vec3 GizmoRenderer::ScreenToRayDirection(float x, float y) const {
    float const ndcX    = 2.f * x / static_cast<float>(m_ViewportW) - 1.f;
    float const ndcY    = 1.f - 2.f * y / static_cast<float>(m_ViewportH);
    float const aspect  = static_cast<float>(m_ViewportW) / static_cast<float>(m_ViewportH);
    float const tanHalf = std::tan(radians(m_Camera.GetPerspectiveFOV()) * 0.5f);
    return (m_Camera.GetRightDirection() * (-ndcX * aspect * tanHalf) +
            m_Camera.GetUpDirection() * (ndcY * tanHalf) +
            m_Camera.GetFrontDirection())
        .normalize();
}

Vec3 GizmoRenderer::RayAxisClosestPoint(Vec3 P, Vec3 D, Vec3 Q, Vec3 A) {
    Vec3 const w = P - Q;
    float b = A.dotProduct(D), d = A.dotProduct(w), e = D.dotProduct(w);
    float const den = 1.f - b * b;
    if (std::abs(den) < 1e-6f) { return Q; }
    return Q + A * ((d - b * e) / den);
}

bool GizmoRenderer::RayPlaneIntersect(Vec3 rayOrigin, Vec3 rayDir, Vec3 planePoint,
                                      Vec3 planeNormal, float& outT) {
    float const denom = rayDir.dotProduct(planeNormal);
    if (std::abs(denom) < 0.08f) {
        return false;  // ~5° grazing threshold
    }
    outT = (planePoint - rayOrigin).dotProduct(planeNormal) / denom;
    return outT >= 0.f;
}

// ---- hit testing ------------------------------------------------------------

GizmoAxis GizmoRenderer::HitTestAxes(float mouseX, float mouseY) {
    auto ctx = ActiveGizmoCtx();
    if (!ctx) { return GizmoAxis::None; }

    Matx4f const VP = m_Camera.GetViewProjectionMatrix();
    auto W = static_cast<float>(m_ViewportW), H = static_cast<float>(m_ViewportH);
    Vec2 const sc = worldToScreen(ctx->pos, VP, W, H);

    constexpr float kThresh                 = 8.f;
    constexpr std::array<GizmoAxis, 3> axes = {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z};
    float best                              = kThresh;
    GizmoAxis hit                           = GizmoAxis::None;

    auto const& tc = m_Scene.GetComponent<TransformComponent>(ctx->entity);
    for (GizmoAxis const axis : axes) {
        Vec2 const end = worldToScreen(ctx->pos + AxisDir(axis, tc) * ctx->scale, VP, W, H);
        float const d  = segDist2D(mouseX, mouseY, sc.x, sc.y, end.x, end.y);
        if (d < best) {
            best = d;
            hit  = axis;
        }
    }
    return hit;
}

GizmoAxis GizmoRenderer::HitTestRings(float mouseX, float mouseY) {
    auto ctx = ActiveGizmoCtx();
    if (!ctx) { return GizmoAxis::None; }

    Vec3 const rayOrigin = m_Camera.GetPosition();
    Vec3 const rayDir    = ScreenToRayDirection(mouseX, mouseY);
    float const tol      = ctx->scale * 0.18f;

    constexpr std::array<GizmoAxis, 3> axes = {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z};
    float best                              = tol;
    GizmoAxis hit                           = GizmoAxis::None;

    auto const& tc = m_Scene.GetComponent<TransformComponent>(ctx->entity);
    for (GizmoAxis const axis : axes) {
        float t = NAN;
        if (!RayPlaneIntersect(rayOrigin, rayDir, ctx->pos, AxisDir(axis, tc), t)) { continue; }
        float const err = std::abs((rayOrigin + rayDir * t - ctx->pos).length() - ctx->scale);
        if (err < best) {
            best = err;
            hit  = axis;
        }
    }
    return hit;
}

// ---- interaction ------------------------------------------------------------

bool GizmoRenderer::OnMouseButtonPressed(float vx, float vy) {
    auto ctx = ActiveGizmoCtx();
    if (!ctx) { return false; }

    GizmoAxis const hit =
        (m_Mode == GizmoMode::Rotation) ? HitTestRings(vx, vy) : HitTestAxes(vx, vy);
    if (hit == GizmoAxis::None) { return false; }

    auto const& tc = m_Scene.GetComponent<TransformComponent>(ctx->entity);
    m_DragEntity   = ctx->entity;
    m_DragAxis     = hit;
    m_IsDragging   = true;

    // Snapshot ALL selected entities so multi-entity and history work correctly.
    m_SnapshotTransforms.clear();
    for (entt::entity e : m_SelCtx.GetSelectedEntities()) {
        if (m_Scene.HasComponent<TransformComponent>(e)) {
            m_SnapshotTransforms.emplace_back(e, m_Scene.GetComponent<TransformComponent>(e));
        }
    }

    m_DragPivotWorldPos = ctx->pos;
    Vec3 const ray      = ScreenToRayDirection(vx, vy);
    m_DragAxisDir       = AxisDir(hit, tc);

    if (m_Mode == GizmoMode::Rotation) {
        float t = NAN;
        m_RotDragRefPoint =
            RayPlaneIntersect(m_Camera.GetPosition(), ray, ctx->pos, m_DragAxisDir, t)
                ? m_Camera.GetPosition() + ray * t
                : ctx->pos;
        ringRefVectors(hit, m_RotDragRefU, m_RotDragRefV);
        if (m_Space == GizmoSpace::Local) {
            Matx4f const entRot =
                Matx4f::rotation(tc.EulerDegrees.x, tc.EulerDegrees.y, tc.EulerDegrees.z);
            Vec4 const u  = entRot * Vec4(m_RotDragRefU.x, m_RotDragRefU.y, m_RotDragRefU.z, 0.f);
            Vec4 const v  = entRot * Vec4(m_RotDragRefV.x, m_RotDragRefV.y, m_RotDragRefV.z, 0.f);
            m_RotDragRefU = {u.x, u.y, u.z};
            m_RotDragRefV = {v.x, v.y, v.z};
        }
    } else {
        m_DragStartHitPt =
            RayAxisClosestPoint(m_Camera.GetPosition(), ray, ctx->pos, m_DragAxisDir);
        if (m_Mode == GizmoMode::Scale) {
            m_DragStartDist = (m_DragStartHitPt - ctx->pos).dotProduct(m_DragAxisDir);
        }
    }
    return true;
}

bool GizmoRenderer::OnMouseButtonReleased() {
    if (!m_IsDragging) { return false; }

    if ((m_HistSys != nullptr) && !m_SnapshotTransforms.empty()) {
        std::vector<std::pair<entt::entity, TransformComponent>> afterStates;
        bool anyChanged = false;
        for (auto const& [entity, before] : m_SnapshotTransforms) {
            if (!m_Scene.HasComponent<TransformComponent>(entity)) { continue; }
            auto const& after = m_Scene.GetComponent<TransformComponent>(entity);
            if (after.GetMatrix() != before.GetMatrix()) { anyChanged = true; }
            afterStates.emplace_back(entity, after);
        }
        if (anyChanged) {
            m_HistSys->Push(std::make_unique<MultiTransformCommand>(&m_Scene, m_SnapshotTransforms,
                                                                    std::move(afterStates)));
        }
    }

    m_IsDragging = false;
    m_DragAxis   = GizmoAxis::None;
    m_DragEntity = entt::null;
    m_SnapshotTransforms.clear();
    return true;
}

void GizmoRenderer::ApplyTranslationDrag(Vec3 ray, bool snap) {
    Vec3 const newHit =
        RayAxisClosestPoint(m_Camera.GetPosition(), ray, m_DragPivotWorldPos, m_DragAxisDir);
    float disp = (newHit - m_DragStartHitPt).dotProduct(m_DragAxisDir);
    if (snap) { disp = std::round(disp / m_Snap.translate) * m_Snap.translate; }
    Vec3 const delta = m_DragAxisDir * disp;
    forEachSnapshot(m_Scene, m_SnapshotTransforms,
                    [&](TransformComponent& tc, TransformComponent const& startTC) {
                        tc.Translation = startTC.Translation + delta;
                    });
}

void GizmoRenderer::ApplyScaleDrag(Vec3 ray, bool snap) {
    Vec3 const newHit =
        RayAxisClosestPoint(m_Camera.GetPosition(), ray, m_DragPivotWorldPos, m_DragAxisDir);
    float const newDist = (newHit - m_DragPivotWorldPos).dotProduct(m_DragAxisDir);
    if (std::abs(m_DragStartDist) < 1e-4f) { return; }
    float const factor        = newDist / m_DragStartDist;
    float Vec3::* const field = axisToField(m_DragAxis);
    if (field == nullptr) { return; }
    bool const sharedPivot = (m_PivotMode != PivotMode::IndividualOrigins);
    auto applyS            = [&](float base) {
        float v = base * factor;
        return snap ? std::round(v / m_Snap.scale) * m_Snap.scale : v;
    };
    forEachSnapshot(
        m_Scene, m_SnapshotTransforms,
        [&](TransformComponent& tc, TransformComponent const& startTC) {
            tc.Scale.*field = applyS(startTC.Scale.*field);
            if (sharedPivot) {
                float const axisComp =
                    (startTC.Translation - m_DragPivotWorldPos).dotProduct(m_DragAxisDir);
                tc.Translation = startTC.Translation + m_DragAxisDir * (axisComp * (factor - 1.f));
            } else {
                tc.Translation = startTC.Translation;
            }
        });
}

void GizmoRenderer::ApplyRotationDrag(Vec3 ray, bool snap) {
    float t = NAN;
    if (!RayPlaneIntersect(m_Camera.GetPosition(), ray, m_DragPivotWorldPos, m_DragAxisDir, t)) {
        return;
    }
    Vec3 const newHit = m_Camera.GetPosition() + ray * t;
    Vec3 const refVec = m_RotDragRefPoint - m_DragPivotWorldPos;
    Vec3 const newVec = newHit - m_DragPivotWorldPos;
    float delta = (std::atan2(newVec.dotProduct(m_RotDragRefV), newVec.dotProduct(m_RotDragRefU)) -
                   std::atan2(refVec.dotProduct(m_RotDragRefV), refVec.dotProduct(m_RotDragRefU))) *
                  (180.f / PI_F);
    if (snap) { delta = std::round(delta / m_Snap.rotate) * m_Snap.rotate; }

    if (m_DragAxis == GizmoAxis::None) { return; }
    // Y ring atan2 convention is opposite to right-hand Y rotation; flip sign so orbit and
    // orientation both go in the direction the user dragged.
    float const sign     = (m_DragAxis == GizmoAxis::Y) ? -1.f : 1.f;
    float const deltaRad = delta * (PI_F / 180.f);
    // Single signed rotation matrix used for both orientation and position-orbit so they stay
    // consistent in direction (using separate matrices with different signs broke Y-axis orbit).
    Matx4f const rot       = Matx4f::rotation(m_DragAxisDir, sign * deltaRad);
    bool const sharedPivot = (m_PivotMode != PivotMode::IndividualOrigins);
    forEachSnapshot(m_Scene, m_SnapshotTransforms,
                    [&](TransformComponent& tc, TransformComponent const& startTC) {
                        // Pre-multiply by the world-space delta rotation so all entities rotate
                        // around the same gizmo axis, regardless of their individual orientations.
                        Matx4f const startRot = Matx4f::rotation(
                            startTC.EulerDegrees.x, startTC.EulerDegrees.y, startTC.EulerDegrees.z);
                        tc.EulerDegrees = matToEulerDegrees(rot * startRot);
                        if (sharedPivot) {
                            Vec3 const offset  = startTC.Translation - m_DragPivotWorldPos;
                            Vec4 const rotated = rot * Vec4(offset.x, offset.y, offset.z, 0.f);
                            tc.Translation =
                                m_DragPivotWorldPos + Vec3{rotated.x, rotated.y, rotated.z};
                        } else {
                            tc.Translation = startTC.Translation;
                        }
                    });
}

bool GizmoRenderer::OnMouseMoved(float vx, float vy) {
    if (!m_IsDragging) {
        m_HoveredAxis =
            (m_Mode == GizmoMode::Rotation) ? HitTestRings(vx, vy) : HitTestAxes(vx, vy);
        return false;
    }

    if (m_SelCtx.GetSelectedEntities().empty()) {
        m_IsDragging = false;
        return false;
    }

    Vec3 const ray  = ScreenToRayDirection(vx, vy);
    bool const snap = Input::IsKeyPressed(KeyCode::LeftControl);

    if (m_Mode == GizmoMode::Translation) {
        ApplyTranslationDrag(ray, snap);
    } else if (m_Mode == GizmoMode::Scale) {
        ApplyScaleDrag(ray, snap);
    } else {
        ApplyRotationDrag(ray, snap);
    }
    return true;
}
