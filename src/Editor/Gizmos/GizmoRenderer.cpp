#include "Editor/Gizmos/GizmoRenderer.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Core/Systems/Commands/TransformCommand.hpp"
#include "Core/Components/Component.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Camera.hpp"
#include "Platform/Graphics/RenderCommand.hpp"
#include <cmath>
#include <algorithm>

// ---- file-scope helpers -----------------------------------------------------

static Vec3 axisToVec3(GizmoAxis a) {
    switch (a) {
        case GizmoAxis::X: return {1.f, 0.f, 0.f};
        case GizmoAxis::Y: return {0.f, 1.f, 0.f};
        case GizmoAxis::Z: return {0.f, 0.f, 1.f};
        default:           return {0.f, 0.f, 0.f};
    }
}

static float segDist2D(float px, float py, float ax, float ay, float bx, float by) {
    float abx = bx-ax, aby = by-ay, apx = px-ax, apy = py-ay;
    float ab2 = abx*abx + aby*aby;
    float t   = (ab2 > 1e-12f) ? std::clamp((apx*abx + apy*aby) / ab2, 0.f, 1.f) : 0.f;
    float dx  = px - (ax + t*abx), dy = py - (ay + t*aby);
    return std::sqrt(dx*dx + dy*dy);
}

static Vec2 worldToScreen(const Vec3& w, const Matx4f& VP, float W, float H) {
    Vec4 clip = VP * Vec4(w.x, w.y, w.z, 1.f);
    if (std::abs(clip.w) < 1e-6f) return {-9999.f, -9999.f};
    float nx = clip.x / clip.w, ny = clip.y / clip.w;
    return {(nx*0.5f + 0.5f)*W, (1.f - (ny*0.5f + 0.5f))*H};
}

static void ringRefVectors(GizmoAxis axis, Vec3& outU, Vec3& outV) {
    switch (axis) {
        case GizmoAxis::X: outU = {0,1,0}; outV = {0,0,1}; break;
        case GizmoAxis::Y: outU = {1,0,0}; outV = {0,0,1}; break;
        case GizmoAxis::Z: outU = {1,0,0}; outV = {0,1,0}; break;
        default:           outU = {1,0,0}; outV = {0,0,1}; break;
    }
}

// ---- GizmoRenderer ----------------------------------------------------------

GizmoRenderer::GizmoRenderer(Scene& scene, SelectionContext& selCtx, const Camera& camera)
    : m_Scene(scene), m_SelCtx(selCtx), m_Camera(camera) {}

GizmoRenderer::~GizmoRenderer() = default;

void GizmoRenderer::OnAttach() {
    m_ArrowMesh  = Mesh::CreateArrow();
    // Thin torus in XZ plane (Y = hole axis), unit major radius; re-oriented per ring at draw time.
    m_RingMesh   = Mesh::CreateTorus(1.0f, 0.035f, 64, 10);
    m_CenterMesh = Mesh::CreateSphere(1.0f, 8, 8);
    m_ConeMesh   = Mesh::CreateCone();
    m_HistSys    = m_Scene.GetSystem<HistorySystem>();
}

void GizmoRenderer::SetViewportSize(uint32_t w, uint32_t h) { m_ViewportW = w; m_ViewportH = h; }

// ---- context ----------------------------------------------------------------

std::optional<GizmoRenderer::GizmoCtx> GizmoRenderer::ActiveGizmoCtx() {
    const auto& sel = m_SelCtx.GetSelectedEntities();
    if (sel.empty()) return std::nullopt;
    entt::entity e = *sel.begin();
    if (!m_Scene.HasComponent<TransformComponent>(e)) return std::nullopt;
    Vec3  pos   = m_GlobalTransform.transformPoint(m_Scene.GetComponent<TransformComponent>(e).Translation);
    float scale = (pos - m_Camera.GetPosition()).length() * 0.15f;
    return GizmoCtx{e, pos, scale};
}

// ---- rendering --------------------------------------------------------------

void GizmoRenderer::Draw() {
    auto ctx = ActiveGizmoCtx();
    if (!ctx) return;

    auto colorFor = [&](GizmoAxis axis, Vec4 base) -> Vec4 {
        if (m_HoveredAxis != axis && m_DragAxis != axis) return base;
        return {std::min(base.x*1.6f,1.f), std::min(base.y*1.6f,1.f), std::min(base.z*1.6f,1.f), 1.f};
    };

    RenderCommand::SetDepthFunc(DepthFunc::Always);

    if (m_Mode == GizmoMode::Translation) {
        Renderer::SubmitFlat(m_ArrowMesh, colorFor(GizmoAxis::X, {1.f,0.2f,0.2f,1.f}),
            Matx4f::translation(ctx->pos) * Matx4f::scalingScalar(ctx->scale));
        Renderer::SubmitFlat(m_ArrowMesh, colorFor(GizmoAxis::Y, {0.2f,1.f,0.2f,1.f}),
            Matx4f::translation(ctx->pos) * Matx4f::rotationZ(90.f) * Matx4f::scalingScalar(ctx->scale));
        Renderer::SubmitFlat(m_ArrowMesh, colorFor(GizmoAxis::Z, {0.2f,0.2f,1.f,1.f}),
            Matx4f::translation(ctx->pos) * Matx4f::rotationY(-90.f) * Matx4f::scalingScalar(ctx->scale));
    } else {
        // Base torus lies in XZ plane; each ring re-oriented via a rotation before the uniform scale.
        // Two cones per ring mark the tangent at ±(1,0,0) in ring-local space.
        constexpr float coneR = 0.12f, coneH = 0.28f;
        struct RingDef { GizmoAxis axis; Vec4 color; Matx4f rot; };
        RingDef rings[3] = {
            {GizmoAxis::X, {1.f,0.2f,0.2f,1.f}, Matx4f::rotationZ(90.f)},
            {GizmoAxis::Y, {0.2f,1.f,0.2f,1.f}, Matx4f::identity()      },
            {GizmoAxis::Z, {0.2f,0.2f,1.f,1.f}, Matx4f::rotationX(90.f)},
        };
        Matx4f cs = Matx4f::scaling(Vec3{coneR, coneR, coneH});
        for (const auto& ring : rings) {
            Vec4   col  = colorFor(ring.axis, ring.color);
            Matx4f base = Matx4f::translation(ctx->pos) * ring.rot * Matx4f::scalingScalar(ctx->scale);
            Renderer::SubmitFlat(m_RingMesh, col, base);
            Renderer::SubmitFlat(m_ConeMesh, col, base * Matx4f::translation(Vec3{ 1,0,0}) * cs);
            Renderer::SubmitFlat(m_ConeMesh, col, base * Matx4f::translation(Vec3{-1,0,0}) * Matx4f::rotationY(180.f) * cs);
        }
    }

    Renderer::SubmitFlat(m_CenterMesh, {0.9f,0.9f,0.9f,1.f},
        Matx4f::translation(ctx->pos) * Matx4f::scalingScalar(ctx->scale * 0.08f));

    RenderCommand::SetDepthFunc(DepthFunc::Less);
}

// ---- math helpers -----------------------------------------------------------

Vec3 GizmoRenderer::ScreenToRayDirection(float x, float y) const {
    float ndcX    = 2.f * x / static_cast<float>(m_ViewportW) - 1.f;
    float ndcY    = 1.f - 2.f * y / static_cast<float>(m_ViewportH);
    float aspect  = static_cast<float>(m_ViewportW) / static_cast<float>(m_ViewportH);
    float tanHalf = std::tan(radians(m_Camera.GetPerspectiveFOV()) * 0.5f);
    return (m_Camera.GetRightDirection() * (-ndcX * aspect * tanHalf)
          + m_Camera.GetUpDirection()    * ( ndcY * tanHalf)
          + m_Camera.GetFrontDirection()).normalize();
}

Vec3 GizmoRenderer::RayAxisClosestPoint(Vec3 P, Vec3 D, Vec3 Q, Vec3 A) const {
    Vec3  w = P - Q;
    float b = A.dotProduct(D), d = A.dotProduct(w), e = D.dotProduct(w);
    float den = 1.f - b*b;
    if (std::abs(den) < 1e-6f) return Q;
    return Q + A * ((d - b*e) / den);
}

bool GizmoRenderer::RayPlaneIntersect(Vec3 rayOrigin, Vec3 rayDir,
                                       Vec3 planePoint, Vec3 planeNormal, float& outT) const {
    float denom = rayDir.dotProduct(planeNormal);
    if (std::abs(denom) < 0.08f) return false;  // ~5° grazing threshold
    outT = (planePoint - rayOrigin).dotProduct(planeNormal) / denom;
    return outT >= 0.f;
}

// ---- hit testing ------------------------------------------------------------

GizmoAxis GizmoRenderer::HitTestAxes(float mouseX, float mouseY) {
    auto ctx = ActiveGizmoCtx();
    if (!ctx) return GizmoAxis::None;

    Matx4f VP = m_Camera.GetViewProjectionMatrix();
    float  W  = static_cast<float>(m_ViewportW), H = static_cast<float>(m_ViewportH);
    Vec2   sc = worldToScreen(ctx->pos, VP, W, H);

    constexpr float     kThresh = 8.f;
    constexpr GizmoAxis axes[3] = {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z};
    float     best = kThresh;
    GizmoAxis hit  = GizmoAxis::None;

    for (GizmoAxis axis : axes) {
        Vec2  end = worldToScreen(ctx->pos + axisToVec3(axis) * ctx->scale, VP, W, H);
        float d   = segDist2D(mouseX, mouseY, sc.x, sc.y, end.x, end.y);
        if (d < best) { best = d; hit = axis; }
    }
    return hit;
}

GizmoAxis GizmoRenderer::HitTestRings(float mouseX, float mouseY) {
    auto ctx = ActiveGizmoCtx();
    if (!ctx) return GizmoAxis::None;

    Vec3  rayOrigin = m_Camera.GetPosition();
    Vec3  rayDir    = ScreenToRayDirection(mouseX, mouseY);
    float tol       = ctx->scale * 0.18f;

    constexpr GizmoAxis axes[3] = {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z};
    float     best = tol;
    GizmoAxis hit  = GizmoAxis::None;

    for (GizmoAxis axis : axes) {
        float t;
        if (!RayPlaneIntersect(rayOrigin, rayDir, ctx->pos, axisToVec3(axis), t)) continue;
        float err = std::abs((rayOrigin + rayDir*t - ctx->pos).length() - ctx->scale);
        if (err < best) { best = err; hit = axis; }
    }
    return hit;
}

// ---- interaction ------------------------------------------------------------

bool GizmoRenderer::OnMouseButtonPressed(float vx, float vy) {
    auto ctx = ActiveGizmoCtx();
    if (!ctx) return false;

    GizmoAxis hit = (m_Mode == GizmoMode::Translation)
        ? HitTestAxes(vx, vy) : HitTestRings(vx, vy);
    if (hit == GizmoAxis::None) return false;

    auto& tc               = m_Scene.GetComponent<TransformComponent>(ctx->entity);
    m_DragEntity           = ctx->entity;
    m_TransformAtDragStart = tc;
    m_DragAxis             = hit;
    m_IsDragging           = true;

    Vec3 ray = ScreenToRayDirection(vx, vy);
    if (m_Mode == GizmoMode::Translation) {
        m_DragStartHitPt = RayAxisClosestPoint(m_Camera.GetPosition(), ray, ctx->pos, axisToVec3(hit));
    } else {
        float t;
        m_RotDragRefPoint = RayPlaneIntersect(m_Camera.GetPosition(), ray, ctx->pos, axisToVec3(hit), t)
            ? m_Camera.GetPosition() + ray * t
            : ctx->pos;
        ringRefVectors(hit, m_RotDragRefU, m_RotDragRefV);
    }
    return true;
}

bool GizmoRenderer::OnMouseButtonReleased() {
    if (!m_IsDragging) return false;

    if (m_HistSys && m_DragEntity != entt::null
            && m_Scene.HasComponent<TransformComponent>(m_DragEntity)) {
        const auto& after = m_Scene.GetComponent<TransformComponent>(m_DragEntity);
        if (after.GetMatrix() != m_TransformAtDragStart.GetMatrix())
            m_HistSys->Push(std::make_unique<TransformCommand>(
                &m_Scene, m_DragEntity, m_TransformAtDragStart, after));
    }
    m_IsDragging = false;
    m_DragAxis   = GizmoAxis::None;
    m_DragEntity = entt::null;
    return true;
}

bool GizmoRenderer::OnMouseMoved(float vx, float vy) {
    if (!m_IsDragging) {
        m_HoveredAxis = (m_Mode == GizmoMode::Translation)
            ? HitTestAxes(vx, vy) : HitTestRings(vx, vy);
        return false;
    }

    if (m_SelCtx.GetSelectedEntities().empty()) { m_IsDragging = false; return false; }

    auto& tc  = m_Scene.GetComponent<TransformComponent>(m_DragEntity);
    Vec3  pos = m_GlobalTransform.transformPoint(tc.Translation);
    Vec3  ray = ScreenToRayDirection(vx, vy);

    if (m_Mode == GizmoMode::Translation) {
        Vec3 newHit  = RayAxisClosestPoint(m_Camera.GetPosition(), ray, pos, axisToVec3(m_DragAxis));
        tc.Translation  += newHit - m_DragStartHitPt;
        m_DragStartHitPt = newHit;
    } else {
        float t;
        if (!RayPlaneIntersect(m_Camera.GetPosition(), ray, pos, axisToVec3(m_DragAxis), t))
            return true;
        Vec3  newHit = m_Camera.GetPosition() + ray * t;
        Vec3  refVec = m_RotDragRefPoint - pos;
        Vec3  newVec = newHit             - pos;
        float delta  = (std::atan2(newVec.dotProduct(m_RotDragRefV), newVec.dotProduct(m_RotDragRefU))
                      - std::atan2(refVec.dotProduct(m_RotDragRefV), refVec.dotProduct(m_RotDragRefU)))
                      * (180.f / PI_F);
        if      (m_DragAxis == GizmoAxis::X) tc.EulerDegrees.x = m_TransformAtDragStart.EulerDegrees.x + delta;
        // Y rotation (right-hand rule) takes +X toward -Z, opposite to the atan2(z,x) sense.
        else if (m_DragAxis == GizmoAxis::Y) tc.EulerDegrees.y = m_TransformAtDragStart.EulerDegrees.y - delta;
        else if (m_DragAxis == GizmoAxis::Z) tc.EulerDegrees.z = m_TransformAtDragStart.EulerDegrees.z + delta;
    }
    return true;
}
