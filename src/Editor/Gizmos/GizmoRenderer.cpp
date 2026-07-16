#include "Editor/Gizmos/GizmoRenderer.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Core/Systems/Commands/TransformCommand.hpp"
#include "Core/Components/Component.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Camera.hpp"
#include "Platform/Graphics/Vertex.hpp"
#include "Platform/Graphics/RenderCommand.hpp"
#include <cmath>
#include <numbers>
#include <vector>
#include <algorithm>

// ---- internal helpers -------------------------------------------------------

static Vec3 axisToVec3(GizmoAxis a) {
    switch (a) {
        case GizmoAxis::X: return {1.0f, 0.0f, 0.0f};
        case GizmoAxis::Y: return {0.0f, 1.0f, 0.0f};
        case GizmoAxis::Z: return {0.0f, 0.0f, 1.0f};
        default:           return {0.0f, 0.0f, 0.0f};
    }
}

// 2D distance from point P to segment AB.
static float segDist2D(float px, float py,
                       float ax, float ay,
                       float bx, float by) {
    float abx = bx - ax, aby = by - ay;
    float apx = px - ax, apy = py - ay;
    float ab2 = abx * abx + aby * aby;
    float t = (ab2 > 1e-12f) ? std::clamp((apx * abx + apy * aby) / ab2, 0.0f, 1.0f) : 0.0f;
    float cx = ax + t * abx, cy = ay + t * aby;
    float dx = px - cx,      dy = py - cy;
    return std::sqrt(dx * dx + dy * dy);
}

// Project a world point through the VP matrix into screen pixel coordinates.
static Vec2 worldToScreen(const Vec3& w, const Matx4f& VP, float W, float H) {
    Vec4 clip = VP * Vec4(w.x, w.y, w.z, 1.0f);
    if (std::abs(clip.w) < 1e-6f) return {-9999.0f, -9999.0f};
    float ndcX = clip.x / clip.w;
    float ndcY = clip.y / clip.w;
    return {
        (ndcX * 0.5f + 0.5f) * W,
        (1.0f - (ndcY * 0.5f + 0.5f)) * H
    };
}

// ---- arrow mesh generation --------------------------------------------------

std::shared_ptr<Mesh> GizmoRenderer::BuildArrowMesh() {
    // Single arrow pointing along +X, unit length [0, 1].
    // Shaft: 12-sided cylinder, radius r, length 0 → shaftLen.
    // Cone:  12-sided cone, base radius R, from shaftLen → 1.0.

    constexpr int   N        = 12;
    constexpr float r        = 0.03f;
    constexpr float shaftLen = 0.75f;
    constexpr float R        = 0.08f;
    constexpr float totalLen = 1.0f;
    constexpr float TWO_PI   = 2.0f * std::numbers::pi_v<float>;

    std::vector<Vertex>   verts;
    std::vector<uint32_t> idx;

    auto addVert = [&](Vec3 pos, Vec3 normal) {
        verts.push_back({pos, normal, {0.0f, 0.0f}});
    };

    // ---- shaft ----------------------------------------------------------------
    // bottom ring [0, N)
    for (int i = 0; i < N; ++i) {
        float theta = TWO_PI * i / N;
        float cy = r * std::cos(theta), cz = r * std::sin(theta);
        addVert({0.0f, cy, cz}, {0.0f, std::cos(theta), std::sin(theta)});
    }
    // top ring [N, 2N)
    for (int i = 0; i < N; ++i) {
        float theta = TWO_PI * i / N;
        float cy = r * std::cos(theta), cz = r * std::sin(theta);
        addVert({shaftLen, cy, cz}, {0.0f, std::cos(theta), std::sin(theta)});
    }
    // sides: 2N triangles
    for (int i = 0; i < N; ++i) {
        uint32_t b0 = i,         b1 = (i + 1) % N;
        uint32_t t0 = N + i,     t1 = N + (i + 1) % N;
        idx.insert(idx.end(), {b0, t0, t1, b0, t1, b1});
    }

    // bottom cap [2N, 3N+1)
    {
        uint32_t center = static_cast<uint32_t>(verts.size());
        addVert({0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f});
        uint32_t rimBase = static_cast<uint32_t>(verts.size());
        for (int i = 0; i < N; ++i) {
            float theta = TWO_PI * i / N;
            addVert({0.0f, r * std::cos(theta), r * std::sin(theta)}, {-1.0f, 0.0f, 0.0f});
        }
        for (int i = 0; i < N; ++i)
            idx.insert(idx.end(), {center, rimBase + (i + 1) % N, rimBase + i});
    }

    // top cap [3N+1, 4N+2)
    {
        uint32_t center = static_cast<uint32_t>(verts.size());
        addVert({shaftLen, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
        uint32_t rimBase = static_cast<uint32_t>(verts.size());
        for (int i = 0; i < N; ++i) {
            float theta = TWO_PI * i / N;
            addVert({shaftLen, r * std::cos(theta), r * std::sin(theta)}, {1.0f, 0.0f, 0.0f});
        }
        for (int i = 0; i < N; ++i)
            idx.insert(idx.end(), {center, rimBase + i, rimBase + (i + 1) % N});
    }

    // ---- cone -----------------------------------------------------------------
    // sides: N triangles, 3 unique verts each (flat shading)
    for (int i = 0; i < N; ++i) {
        float t0 = TWO_PI * i / N, t1 = TWO_PI * (i + 1) / N;
        Vec3 v0 = {shaftLen, R * std::cos(t0), R * std::sin(t0)};
        Vec3 v1 = {shaftLen, R * std::cos(t1), R * std::sin(t1)};
        Vec3 v2 = {totalLen, 0.0f, 0.0f};
        Vec3 e1 = v1 - v0, e2 = v2 - v0;
        Vec3 n  = e1.crossProduct(e2).normalize();
        uint32_t base = static_cast<uint32_t>(verts.size());
        addVert(v0, n); addVert(v1, n); addVert(v2, n);
        idx.insert(idx.end(), {base, base + 1, base + 2});
    }

    // cone base cap
    {
        uint32_t center = static_cast<uint32_t>(verts.size());
        addVert({shaftLen, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f});
        uint32_t rimBase = static_cast<uint32_t>(verts.size());
        for (int i = 0; i < N; ++i) {
            float theta = TWO_PI * i / N;
            addVert({shaftLen, R * std::cos(theta), R * std::sin(theta)}, {-1.0f, 0.0f, 0.0f});
        }
        for (int i = 0; i < N; ++i)
            idx.insert(idx.end(), {center, rimBase + (i + 1) % N, rimBase + i});
    }

    return Mesh::CreateMeshFromData(
        verts.data(),
        static_cast<uint32_t>(verts.size() * sizeof(Vertex)),
        idx.data(),
        static_cast<uint32_t>(idx.size())
    );
}

std::shared_ptr<Mesh> GizmoRenderer::BuildCenterMesh() {
    return Mesh::CreateSphere(1.0f, 8, 8);
}

// ---- GizmoRenderer ----------------------------------------------------------

GizmoRenderer::GizmoRenderer(Scene& scene, SelectionContext& selCtx, const Camera& camera)
    : m_Scene(scene), m_SelCtx(selCtx), m_Camera(camera) {}

GizmoRenderer::~GizmoRenderer() = default;

void GizmoRenderer::OnAttach() {
    m_ArrowMesh  = BuildArrowMesh();
    m_CenterMesh = BuildCenterMesh();
    m_HistSys    = m_Scene.GetSystem<HistorySystem>();
}

void GizmoRenderer::SetViewportSize(uint32_t w, uint32_t h) {
    m_ViewportW = w;
    m_ViewportH = h;
}

// ---- rendering --------------------------------------------------------------

void GizmoRenderer::Draw() {
    const auto& selected = m_SelCtx.GetSelectedEntities();
    if (selected.empty()) return;

    entt::entity entity = *selected.begin();
    if (!m_Scene.HasComponent<TransformComponent>(entity)) return;

    auto& tc = m_Scene.GetComponent<TransformComponent>(entity);
    Vec3 gizmoPos = m_GlobalTransform.transformPoint(tc.Translation);

    Vec3  diff  = gizmoPos - m_Camera.GetPosition();
    float dist  = diff.length();
    float scale = dist * 0.15f;

    auto colorFor = [&](GizmoAxis axis, Vec4 base) -> Vec4 {
        if (m_HoveredAxis == axis || m_DragAxis == axis)
            return {std::min(base.x * 1.6f, 1.0f),
                    std::min(base.y * 1.6f, 1.0f),
                    std::min(base.z * 1.6f, 1.0f), 1.0f};
        return base;
    };

    // VP is already in s_SceneData from RenderingSystem::BeginScene — calling BeginScene again
    // would clear the framebuffer and erase the scene geometry.
    RenderCommand::SetDepthFunc(DepthFunc::Always);

    Renderer::SubmitFlat(m_ArrowMesh,
        colorFor(GizmoAxis::X, {1.0f, 0.2f, 0.2f, 1.0f}),
        Matx4f::translation(gizmoPos) * Matx4f::scalingScalar(scale));

    Renderer::SubmitFlat(m_ArrowMesh,
        colorFor(GizmoAxis::Y, {0.2f, 1.0f, 0.2f, 1.0f}),
        Matx4f::translation(gizmoPos) * Matx4f::rotationZ(90.0f) * Matx4f::scalingScalar(scale));

    Renderer::SubmitFlat(m_ArrowMesh,
        colorFor(GizmoAxis::Z, {0.2f, 0.2f, 1.0f, 1.0f}),
        Matx4f::translation(gizmoPos) * Matx4f::rotationY(-90.0f) * Matx4f::scalingScalar(scale));

    Renderer::SubmitFlat(m_CenterMesh,
        {0.9f, 0.9f, 0.9f, 1.0f},
        Matx4f::translation(gizmoPos) * Matx4f::scalingScalar(scale * 0.08f));

    RenderCommand::SetDepthFunc(DepthFunc::Less);
}

// ---- interaction ------------------------------------------------------------

Vec3 GizmoRenderer::ScreenToRayDirection(float x, float y) const {
    float ndcX    = 2.0f * x / static_cast<float>(m_ViewportW) - 1.0f;
    float ndcY    = 1.0f - 2.0f * y / static_cast<float>(m_ViewportH);
    float aspect  = static_cast<float>(m_ViewportW) / static_cast<float>(m_ViewportH);
    float tanHalf = std::tan(radians(m_Camera.GetPerspectiveFOV()) * 0.5f);

    Vec3 right   = m_Camera.GetRightDirection();
    Vec3 up      = m_Camera.GetUpDirection();
    Vec3 front   = m_Camera.GetFrontDirection();

    Vec3 dir = right * (-ndcX * aspect * tanHalf)
             + up    * (ndcY * tanHalf)
             + front;
    return dir.normalize();
}

Vec3 GizmoRenderer::RayAxisClosestPoint(Vec3 P, Vec3 D,
                                         Vec3 Q, Vec3 A) const {
    // Both D and A are unit vectors.
    Vec3  w     = P - Q;
    float b     = A.dotProduct(D);
    float d     = A.dotProduct(w);
    float e     = D.dotProduct(w);
    float denom = 1.0f - b * b;
    if (std::abs(denom) < 1e-6f) return Q;   // ray parallel to axis
    float s = (d - b * e) / denom;
    return Q + A * s;
}

GizmoAxis GizmoRenderer::HitTestAxes(float mouseX, float mouseY) {
    const auto& selected = m_SelCtx.GetSelectedEntities();
    if (selected.empty()) return GizmoAxis::None;

    entt::entity entity = *selected.begin();
    if (!m_Scene.HasComponent<TransformComponent>(entity)) return GizmoAxis::None;

    auto& tc = m_Scene.GetComponent<TransformComponent>(entity);
    Vec3 gizmoPos = m_GlobalTransform.transformPoint(tc.Translation);

    Vec3  diff  = gizmoPos - m_Camera.GetPosition();
    float scale = diff.length() * 0.15f;

    Matx4f VP = m_Camera.GetViewProjectionMatrix();
    auto W = static_cast<float>(m_ViewportW);
    auto H = static_cast<float>(m_ViewportH);

    Vec2 center = worldToScreen(gizmoPos, VP, W, H);

    constexpr float kThreshold = 8.0f;
    float bestDist = kThreshold;
    GizmoAxis bestAxis = GizmoAxis::None;

    constexpr GizmoAxis axes[3] = {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z};
    for (GizmoAxis axis : axes) {
        Vec3 axisDir = axisToVec3(axis);
        Vec3 worldEnd = {gizmoPos.x + axisDir.x * scale,
                         gizmoPos.y + axisDir.y * scale,
                         gizmoPos.z + axisDir.z * scale};
        Vec2 end = worldToScreen(worldEnd, VP, W, H);
        float d = segDist2D(mouseX, mouseY,
                             center.x, center.y,
                             end.x,    end.y);
        if (d < bestDist) { bestDist = d; bestAxis = axis; }
    }
    return bestAxis;
}

bool GizmoRenderer::OnMouseButtonPressed(float vx, float vy) {
    GizmoAxis hit = HitTestAxes(vx, vy);
    if (hit == GizmoAxis::None) return false;

    const auto& selected = m_SelCtx.GetSelectedEntities();
    if (selected.empty()) return false;

    m_DragEntity = *selected.begin();
    auto& tc     = m_Scene.GetComponent<TransformComponent>(m_DragEntity);
    Vec3 gizmoPos = m_GlobalTransform.transformPoint(tc.Translation);

    m_TransformAtDragStart = tc;
    m_DragAxis             = hit;
    m_IsDragging           = true;
    m_DragStartHitPt = RayAxisClosestPoint(
        m_Camera.GetPosition(), ScreenToRayDirection(vx, vy),
        gizmoPos, axisToVec3(hit));
    return true;
}

bool GizmoRenderer::OnMouseButtonReleased() {
    if (!m_IsDragging) return false;

    if (m_HistSys && m_DragEntity != entt::null
            && m_Scene.HasComponent<TransformComponent>(m_DragEntity)) {
        const TransformComponent& after = m_Scene.GetComponent<TransformComponent>(m_DragEntity);
        if (after.GetMatrix() != m_TransformAtDragStart.GetMatrix()) {
            m_HistSys->Push(std::make_unique<TransformCommand>(
                &m_Scene, m_DragEntity, m_TransformAtDragStart, after));
        }
    }

    m_IsDragging = false;
    m_DragAxis   = GizmoAxis::None;
    m_DragEntity = entt::null;
    return true;
}

bool GizmoRenderer::OnMouseMoved(float vx, float vy) {
    if (!m_IsDragging) {
        m_HoveredAxis = HitTestAxes(vx, vy);
        return false;
    }

    const auto& selected = m_SelCtx.GetSelectedEntities();
    if (selected.empty()) { m_IsDragging = false; return false; }

    auto& tc = m_Scene.GetComponent<TransformComponent>(*selected.begin());
    Vec3 gizmoPos = m_GlobalTransform.transformPoint(tc.Translation);
    Vec3 axisVec  = axisToVec3(m_DragAxis);

    Vec3 newHit = RayAxisClosestPoint(
        m_Camera.GetPosition(), ScreenToRayDirection(vx, vy),
        gizmoPos, axisVec);

    Vec3 delta = newHit - m_DragStartHitPt;
    tc.Translation += delta;

    m_DragStartHitPt = newHit;
    return true;
}
