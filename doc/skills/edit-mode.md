# Edit Mode — Data Structures & Implementation Guide

## Data model (`Core/EditMesh/EditMesh.hpp`)
```cpp
struct EditVertex {
    Vec3 position, normal;
    Vec2 texCoord;
    bool selected = false;
};

enum class ElementMode { Vertex, Edge, Face };

class EditMesh {
public:
    std::vector<EditVertex>           vertices;
    std::vector<uint32_t>             indices;       // triangles: every 3 = one face
    ElementMode                       mode = ElementMode::Vertex;
    std::unordered_set<uint32_t>      selectedVertices;
    std::unordered_set<uint32_t>      selectedFaces;  // triangle index = indices[face*3]/3
    std::unordered_set<uint64_t>      selectedEdges;  // packed: PackEdge(a,b)
    bool                              selectionDirty = false;

    bool           IsActive() const;           // m_Entity != entt::null
    entt::entity   GetEntity() const;
    uint32_t       FaceCount() const;          // indices.size() / 3
    static uint64_t PackEdge(uint32_t a, uint32_t b); // (min<<32)|max — order-independent
    void           ClearSelection();
    void           SetEntity(entt::entity);
    void           Clear();                    // resets all; m_Entity = null

private:
    entt::entity m_Entity = entt::null;
};
```

## Entry / Exit (`Core/EditMesh/EditModeSystem`)
- **Enter (Tab pressed):** `EditModeSystem::Enter(entity)` — copies `MeshComponent::MeshAsset` vertex/index data into `EditMesh`; sets active entity.
- **Exit (Tab again):** `EditModeSystem::Exit()` — calls `Mesh::UpdateData(vertices, indices)` to re-upload GPU buffers; calls `EditMesh::Clear()`.
- `EditorLayer` owns `bool m_InEditMode`; Tab toggles it and drives Enter/Exit.

## Picking in edit mode
| Mode | PrimitiveID maps to |
|---|---|
| Face | `PrimitiveID` = triangle index directly |
| Vertex | resolve from `indices[face*3 + 0/1/2]`; pick closest projected vertex |
| Edge | find edge shared between the hit face and adjacent face |

`PickingSystem` switches encoding on `m_InEditMode`:
- Object mode: encodes `entityID + 1` in `ObjectID`.
- Edit mode: encodes `primitiveID` in `PrimitiveID`; `ObjectID` = active entity.

## GPU re-upload
```cpp
mesh->UpdateData(newVertices, newIndices);
```
Only call after all ops for the frame are done — one upload per dirty batch.

## Writing a new edit operation
```cpp
// In Core/EditMesh/EditOps.hpp/.cpp
void DoMyOp(EditMesh& mesh, /* params */) {
    // modify mesh.vertices / mesh.indices
    mesh.selectionDirty = true;  // if selection changed
}
```
Then in `EditModeSystem`: call op, set dirty flag, call `mesh->UpdateData()`.

## Undo
Use `MeshEditCommand` (`Core/Systems/Commands/MeshEditCommand.hpp`):
stores before/after snapshots of `vertices` + `indices`. Push via `HistorySystem::Push`.
```cpp
auto cmd = std::make_unique<MeshEditCommand>(mesh, beforeVerts, beforeIdx, afterVerts, afterIdx);
m_HistorySystem->Push(std::move(cmd));
```

## Element selection keys
| Mode | Container | Key type |
|---|---|---|
| `Vertex` | `selectedVertices` | vertex index (`uint32_t`) |
| `Edge` | `selectedEdges` | `PackEdge(a, b)` (`uint64_t`) |
| `Face` | `selectedFaces` | triangle index (`uint32_t`) |

## Planned edit operations (from `doc/features.md`)
Extrude (E), Inset (I), Loop Cut (Ctrl+R), Bevel (Ctrl+B), Merge (M), Delete (X), Fill (F), Knife (K), Flip Normals (Alt+N), Recalculate Normals (Shift+N), Subdivide (RMB menu), Proportional editing (O).
All implemented as free functions in `Core/EditMesh/EditOps.hpp/.cpp`.
