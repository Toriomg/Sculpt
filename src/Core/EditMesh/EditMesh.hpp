// CPU-side editable mesh state; populated on entering edit mode, written back to GPU on exit.
// Owns per-element selection flags; extended by EditOps when mesh operations are implemented.
#pragma once
#include "Core/Entity.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include <cstdint>
#include <vector>

// Mirrors the GPU Vertex layout but adds a selection flag.
struct EditVertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;
    bool selected = false;
};

// Holds CPU vertex/index data for the entity currently in edit mode.
// Vertices are uniquified from the triangle-soup GPU data so adjacent faces share vertices.
class EditMesh {
public:
    [[nodiscard]] bool IsActive() const { return m_Entity != entt::null; }
    [[nodiscard]] entt::entity GetEntity() const { return m_Entity; }

    // Vertices and indices in the same layout as the GPU mesh.
    std::vector<EditVertex> vertices;
    std::vector<uint32_t> indices;

    // Set/clear the active entity (does NOT touch GPU state — caller manages that).
    void SetEntity(entt::entity entity) { m_Entity = entity; }
    void Clear() {
        m_Entity = entt::null;
        vertices.clear();
        indices.clear();
    }

    [[nodiscard]] uint32_t FaceCount() const { return static_cast<uint32_t>(indices.size() / 3); }

private:
    entt::entity m_Entity = entt::null;
};
