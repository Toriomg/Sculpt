// CPU-side editable mesh state; populated on entering edit mode, written back to GPU on exit.
// Owns per-element selection sets for vertex, edge, and face modes.
#pragma once
#include "Core/Entity.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <vector>

// Mirrors the GPU Vertex layout but adds a selection flag.
struct EditVertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;
    bool selected = false;
};

enum class ElementMode { Vertex, Edge, Face };

// Holds CPU vertex/index data for the entity currently in edit mode.
class EditMesh {
public:
    [[nodiscard]] bool IsActive() const { return m_Entity != entt::null; }
    [[nodiscard]] entt::entity GetEntity() const { return m_Entity; }

    std::vector<EditVertex> vertices;
    std::vector<uint32_t> indices;

    ElementMode mode = ElementMode::Vertex;

    std::unordered_set<uint32_t> selectedVertices;
    std::unordered_set<uint32_t> selectedFaces;
    // Packed edge key: (min(a,b) << 32) | max(a,b) — order-independent.
    std::unordered_set<uint64_t> selectedEdges;

    bool selectionDirty = false;

    static uint64_t PackEdge(uint32_t a, uint32_t b) {
        return (static_cast<uint64_t>(std::min(a, b)) << 32U) |
               static_cast<uint64_t>(std::max(a, b));
    }

    void ClearSelection() {
        selectedVertices.clear();
        selectedFaces.clear();
        selectedEdges.clear();
        selectionDirty = true;
    }

    void SetEntity(entt::entity entity) { m_Entity = entity; }
    void Clear() {
        m_Entity = entt::null;
        vertices.clear();
        indices.clear();
        ClearSelection();
        mode           = ElementMode::Vertex;
        selectionDirty = false;
    }

    [[nodiscard]] uint32_t FaceCount() const { return static_cast<uint32_t>(indices.size() / 3); }

private:
    entt::entity m_Entity = entt::null;
};
