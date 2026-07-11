# Missing Features from old-renderer

## Overview
The current codebase represents a complete architectural rewrite from the `old-renderer` tag (75 commits). While modernizing to C++23 and an EnTT-based ECS system, several features were removed that should be considered for re-implementation.

## 1. Infinite Grid System
**Status**: ❌ Removed

**What it was:**
- Sophisticated infinite grid renderer using ray-casting and multi-scale rendering
- Rendered a full-screen quad with shader ray-tracing to project camera rays onto a ground plane (Y=0)
- Three grid scales with perspective-aware fade:
  - Tiny grid (1 unit spacing)
  - Mid grid (10 unit spacing)
  - Major grid (100 unit spacing)
- Fog-based distance culling to prevent visual clutter at distance

**Implementation details:**
- Class: `InfGrid` (owned grid shader in `std::unique_ptr`)
- Shaders: 
  - Vertex: `res/shaders/vert/grid.vs` - Screen-space quad unprojection to world space
  - Fragment: `res/shaders/frag/grid.fs` - Ray-plane intersection with multi-scale grid rendering
- Method: `Draw(view, projection, cameraPos)` called each frame

**Why it matters:**
- Provides visual reference frame for 3D editing
- Essential for understanding spatial positioning
- Multi-scale approach prevents z-fighting and aliasing

---

## 2. Vertex/Primitive Selection System
**Status**: ❌ Removed

**What it was:**
- Interactive click-to-select system for objects and individual primitives
- Two-pass rendering: normal scene + ID-encoding pass
- Could select entire meshes or drill down to specific primitives (faces/vertices)

**Architecture:**
- `SelectionSystem` class managed the selection pipeline
- Took scene, camera, MVP matrix, and mouse coordinates
- Two key methods:
  - `UpdatePickingPass()` - Render objects with ID encoding
  - `ProcessSelectionClick()` - Sample framebuffer to determine selection

**Infrastructure: PickingTexture**
- Framebuffer-based picking with multiple color attachments:
  - Attachment 0: `uvec3` with ObjectID, PrimitiveID, DrawID
  - Attachment 1: World position as `vec4`
  - Depth texture for proper depth testing
- Methods:
  - `EnableWriting()` / `DisableWriting()` - Toggle framebuffer binding
  - `ReadPixel(x, y)` - Return `PixelInfo` struct with IDs
  - `ReadWorldPosition(x, y)` - Get world coordinates of clicked point

**Shader: Picking.shader**
```glsl
// Vertex: Transform to world space, output MVP position
// Fragment: Encode objectID (uniform), primitive ID (gl_PrimitiveID), output world pos
```

**Why it matters:**
- Core feature for interactive 3D editing
- Allows user to select and manipulate objects
- World position tracking enables gizmo-based transforms

---

## 3. Legacy Component System
**Status**: ⚠️ Replaced (but not 1:1 compatible)

**Old system:**
- `GameObject` class - Entity wrapper with transform
- `MeshRendererComponent` - Attached to GameObjects
- `Component` base class with type info
- Scene managed GameObjects directly

**New system:**
- EnTT registry-based ECS
- `Scene` owns `entt::registry`
- Components are plain structs
- Systems derive from `System` base class

**Impact:**
- More flexible and data-oriented
- Better cache locality
- But requires rewriting old game logic to system-based approach

---

## 4. EditorUI Wrapper
**Status**: ⚠️ Partially removed

**What it was:**
- Simple ImGui wrapper managing frame lifecycle
- `BeginFrame()` / `EndFrame()` methods
- Handled window context attachment

**Current state:**
- ImGui still integrated (vendor code present)
- But no dedicated lifecycle manager in platform layer
- ImGui setup likely scattered across `EditorLayer`

---

## 5. Renderer Architecture Changes
**Status**: Refactored

**Old structure:**
- `Renderer` static API (familiar pattern)
- `RenderCommand` for low-level OpenGL calls
- `RenderContext` and `SelectionContext` for state
- Direct Material/Texture/Shader management

**New structure:**
- `Renderer` now minimal static API
- `RenderCommand` still exists but restructured
- `RenderingSystem` (ECS-based) iterates entities
- Material pairs Shader + Texture handle

**Why changed:**
- ECS integration
- Better separation of concerns
- Asset manager for resource lifecycle

---

## 6. Test Suite
**Status**: ❌ Removed

**Old tests:**
- `TestClearColor` - Basic viewport tests
- `TestTexture2D` - Texture binding
- `TestBatchRendering` - Batch rendering validation
- `test3.cpp/h` - General test framework

**Current state:**
- No test infrastructure
- Manual testing via app iteration

---

## Restoration Priority

### High Priority (Core Features)
1. **Grid System** - Essential for 3D editing UX, straightforward port
2. **Selection System** - Enables object manipulation, heavily used workflow
3. **Picking Shader** - Already present, just needs infrastructure

### Medium Priority (Quality of Life)
4. **Test Suite** - Improve dev iteration speed, prevent regressions

### Low Priority (Nice to Have)
5. **EditorUI Wrapper** - Minor convenience, scattered ImGui calls work fine

---

## Technical Debt Notes

- The `Picking.shader` exists in current code but is completely unused (no PickingTexture or SelectionSystem to use it)
- Grid shaders are gone entirely - would need complete re-implementation
- Selection context types removed - would need to design new serialization format
