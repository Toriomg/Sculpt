# Architecture Gaps Analysis
## Current vs. Planned Architecture (from ClassDiagram3.0.drawio)

### Overview
The current codebase implements the **foundation layers** of the planned architecture but is missing critical **editor-facing features** and **interaction systems**. This document maps what exists vs. what the class diagram shows should exist.

---

## Layer Breakdown

### ✅ IMPLEMENTED LAYERS

#### 1. Platform/Graphics Abstraction Layer (GAL)
**Status**: ✅ **Complete**

| Component | File | Status |
|-----------|------|--------|
| **Mesh** | `src/Renderer/Mesh.h` | ✅ Exists |
| **VertexArray** | `src/Platform/Graphics/Buffers/VertexArray.h` | ✅ Exists |
| **VertexBuffer** | `src/Platform/Graphics/Buffers/VertexBuffer.h` | ✅ Exists |
| **VertexBufferLayout** | `src/Platform/Graphics/Buffers/VertexBufferLayout.h` | ✅ Exists |
| **IndexBuffer** | `src/Platform/Graphics/Buffers/IndexBuffer.h` | ✅ Exists |
| **RenderCommand** | `src/Platform/Graphics/RenderCommand.h` | ✅ Exists |
| **Material** | `src/Renderer/Material.h` | ✅ Exists |
| **Texture** | `src/Platform/Graphics/Texture.h` | ✅ Exists |
| **Shader** | `src/Platform/Graphics/Shader.h` | ✅ Exists |
| **Framebuffer** | `src/Platform/Graphics/Framebuffer.h` | ✅ Exists |
| **Vertex** | `src/Platform/Graphics/Vertex.h` | ✅ Exists |

---

#### 2. Application Layer
**Status**: ✅ **Nearly Complete**

| Component | File | Status |
|-----------|------|--------|
| **Application** | `src/Platform/Application.h` | ✅ Exists |
| **Time** | `src/Platform/System/Time.h` | ✅ Exists |
| **Event** | `src/Platform/System/Events/Event.h` | ✅ Exists |
| **Event Types** | `src/Platform/System/Events/{Keyboard,Mouse,Window}Event.h` | ✅ Exists |
| **Window** | `src/Platform/System/Window/Window.h` | ✅ Exists |
| **Input** | `src/Platform/System/Input.h` | ✅ Exists |
| **KeyCodes** | `src/Platform/System/Input/KeyCodes.h` | ✅ Exists |

**Missing**:
- ❌ **EventDispatcher** - Event routing/binding system not documented

---

#### 3. Core ECS Architecture
**Status**: ✅ **Complete**

| Component | File | Status |
|-----------|------|--------|
| **Scene** | `src/Core/Scene.h` | ✅ Exists (entt::registry owner) |
| **Entity** | `src/Core/Entity.h` | ✅ Exists (alias: entt::entity) |
| **Component** | `src/Core/Components/Component.h` | ✅ Exists (base struct) |
| **System** | `src/Core/Systems/System.h` | ✅ Exists (abstract base) |
| **RenderingSystem** | `src/Core/Systems/RenderingSystem.h` | ✅ Exists |
| **TransformComponent** | `src/Core/Components/Component.h` | ✅ Exists |
| **CameraComponent** | `src/Core/Components/Component.h` | ✅ Exists |
| **MeshComponent** | `src/Core/Components/Component.h` | ✅ Exists |
| **NameComponent** | `src/Core/Components/Component.h` | ✅ Exists |

---

#### 4. Asset Management
**Status**: ✅ **Complete**

| Component | File | Status |
|-----------|------|--------|
| **AssetManager** | `src/AssetManager/AssetManager.h` | ✅ Exists (singleton) |
| **AssetRegistry** | `src/AssetManager/AssetRegistry.h` | ✅ Exists |
| **AssetHandle** | `src/AssetManager/AssetHandle.h` | ✅ Exists |
| **IAsset** | `src/AssetManager/IAsset.h` | ✅ Exists (interface) |
| **IAssetLoader** | `src/AssetManager/Loader/IAssetLoader.h` | ✅ Exists (interface) |
| **LoaderSystem** | `src/AssetManager/Loader/LoaderSystem.h` | ✅ Exists |
| **TextureLoader** | `src/AssetManager/Loader/TextureLoader.h` | ✅ Exists |
| **MeshLoader** | `src/AssetManager/Loader/MeshLoader.h` | ✅ Exists |

---

#### 5. Layer Stack & Platform
**Status**: ✅ **Complete**

| Component | File | Status |
|-----------|------|--------|
| **Layer** | `src/Platform/Layers/Layer.h` | ✅ Exists (base interface) |
| **LayerStack** | `src/Platform/Layers/LayerStack.h` | ✅ Exists |
| **EditorLayer** | `src/Platform/Layers/EditorLayer.h` | ✅ Exists |
| **ServiceLayer** | `src/Platform/Layers/ServiceLayer.h` | ✅ Exists |
| **Renderer** | `src/Renderer/Renderer.h` | ✅ Exists (static API) |
| **Camera** | `src/Renderer/Camera.h` | ✅ Exists |
| **EditorCameraController** | `src/Editor/EditorCameraController.h` | ✅ Exists |

---

## ❌ MISSING LAYERS & SYSTEMS

### Critical Missing Components

#### 1. Selection & Interaction System
**Planned in diagram**: SelectionSystem  
**Current status**: ❌ **Completely Missing**

**Intended purpose:**
- Interactive object/primitive selection via mouse clicks
- Frame-buffer based picking using encoded IDs
- Selection context management (which objects selected, highlight state, etc.)

**Required components:**
- `SelectionSystem` - ECS system managing selection state
- `PickingTexture` - Framebuffer with ID/position attachments
- `PickingShader` - Encode object/primitive IDs (shader exists but unused)
- `PickingFramebuffer` - FBO for off-screen picking pass

**Impact**: Without this, no interactive 3D object manipulation possible. Core feature for editor UX.

**Implementation difficulty**: **Medium** (infrastructure exists: Picking.shader, Framebuffer class)

---

#### 2. Gizmo Rendering System
**Planned in diagram**: GizmoRenderer  
**Current status**: ❌ **Missing**

**Intended purpose:**
- 3D manipulation gizmos (move, rotate, scale)
- Transform visualization and interaction
- Gizmo-to-selection integration

**Required components:**
- `GizmoRenderer` - Main gizmo rendering manager
- Gizmo shaders (move handles, rotation rings, scale boxes)
- `TransformGizmo` - Base gizmo behavior
- `MovementGizmo`, `RotationGizmo`, `ScaleGizmo` - Specific gizmo types

**Impact**: Essential for 3D editing workflow. Can't transform objects without this.

**Implementation difficulty**: **High** (complex geometry + interaction)

**Related to**: SelectionSystem (needs to know what's selected to show gizmo)

---

#### 3. Editor UI System
**Planned in diagram**: EditorUI with Panels  
**Current status**: ❌ **Missing**

**Intended architecture:**
```
EditorUI
├── Module (base panel type)
├── ViewportPanel (3D view)
├── SceneHierarchyPanel (object tree)
└── PropertiesPanel (object properties)
```

**Current state:**
- ImGui integrated (vendor code present)
- EditorLayer contains main loop
- No dedicated UI layer or panel system
- UI logic scattered throughout EditorLayer

**Required components:**
- `EditorUI` - Panel manager
- `Panel` / `Module` - Base panel interface
- `ViewportPanel` - 3D render target + interaction
- `SceneHierarchyPanel` - Scene tree viewer
- `PropertiesPanel` - Inspector for selected object properties

**Impact**: UI/UX polish, organization, maintainability.

**Implementation difficulty**: **High** (ImGui integration + layout management)

**Related to**: SelectionSystem (needs to display selected entity info), AssetManager (for asset browsing panels)

---

#### 4. History/Undo System
**Planned in diagram**: HistorySystem  
**Current status**: ❌ **Missing**

**Intended purpose:**
- Undo/redo functionality for editor operations
- Command pattern implementation
- History stack management

**Required components:**
- `HistorySystem` - ECS system managing undo/redo state
- `Command` - Abstract command interface
- Command implementations (TransformCommand, DeleteEntityCommand, etc.)
- History stack (undo/redo queues)

**Impact**: Professional editor UX, essential for usability.

**Implementation difficulty**: **Medium** (command pattern well-established, needs integration with ECS)

---

#### 5. Task/Job Queue System
**Planned in diagram**: TaskQueue  
**Current status**: ❌ **Missing**

**Intended purpose:**
- Async operations (file loading, asset processing, etc.)
- Job scheduling and execution
- Worker thread pool management

**Required components:**
- `TaskQueue` - Job queue manager
- `Task` - Abstract task interface
- Thread pool for parallel execution
- Task dependencies/ordering

**Impact**: Prevents UI freezing during asset loads, improves responsiveness.

**Implementation difficulty**: **Medium-High** (threading, synchronization concerns)

---

### Minor Missing Components

| Component | Diagram Location | Impact | Difficulty |
|-----------|------------------|--------|-----------|
| **Event Dispatcher** | Application | Event routing between layers | Low |
| **Vertex** (full feature set) | GAL | Already partially exists | Low |

---

## Architecture Readiness Assessment

### What's Production-Ready
- ✅ Graphics rendering foundation (all buffer/shader infrastructure)
- ✅ Asset management and loading
- ✅ ECS structure for game logic
- ✅ Platform abstraction (input, window, time)
- ✅ Basic camera system

### What Blocks Editor Features
- ❌ **Selection** → Can't click objects
- ❌ **Gizmos** → Can't transform objects
- ❌ **UI Panels** → Poor editor usability
- ❌ **Undo/Redo** → No operation history

### Critical Path to Functional Editor

**Phase 1 (MVP)**: Make editor interactive
1. Implement `SelectionSystem` + `PickingTexture` → Can click objects
2. Add basic `GizmoRenderer` → Can move/rotate objects
3. Wire selection feedback to scene → Visual feedback

**Phase 2 (Polish)**: Professional editor
4. Implement `EditorUI` with panels → Organized interface
5. Add `HistorySystem` → Undo/redo
6. Implement `TaskQueue` → Async operations

**Phase 3 (Advanced)**: Optimization
7. Gizmo variants (rotate, scale)
8. Advanced panel features (search, filtering)
9. Custom property editors

---

## Design Notes

### Why These Systems Are Important

**SelectionSystem + GizmoRenderer** form a symbiotic pair:
- SelectionSystem tells you **what** is selected
- GizmoRenderer shows **how to transform** it
- Together they enable the core 3D editing loop: select → manipulate → deselect

**EditorUI Panels** provide:
- **Viewport** - Rendering target + interaction surface
- **Hierarchy** - Scene navigation and organization
- **Properties** - Object data inspection and editing

**HistorySystem** bridges the gap between:
- User actions (transform, delete, create)
- Persistent undo/redo state
- UI feedback (undo button enable/disable)

### Architectural Consistency

All missing systems should follow the established patterns:
- **Systems** should derive from `System` base class and receive `entt::registry`
- **Rendering** should use the `Renderer` static API (BeginScene/EndScene pattern)
- **Components** should be plain structs (TransformComponent, SelectionComponent, etc.)
- **Resources** should use `AssetHandle` for reference counting

### Performance Considerations

- **SelectionSystem**: Picking pass is expensive; consider batching/caching when possible
- **GizmoRenderer**: Gizmos draw on top of scene; use appropriate depth testing
- **TaskQueue**: Thread pool size should scale with CPU cores; avoid thread explosion
- **HistorySystem**: Command storage can grow unbounded; implement history limit or pruning

---

## Recommendation Priority

| System | Priority | Effort | Impact | Recommended For |
|--------|----------|--------|--------|-----------------|
| SelectionSystem | **HIGH** | Medium | Critical | Must-have for editor |
| GizmoRenderer | **HIGH** | High | Critical | Must-have for editor |
| EditorUI | **MEDIUM** | High | Important | Better UX |
| HistorySystem | **MEDIUM** | Medium | Important | Professional feel |
| TaskQueue | **LOW** | Medium | Nice-to-have | Prevents freezes |

---

## File Organization Recommendation

Place new systems in appropriate locations:

```
src/
├── Core/Systems/
│   ├── RenderingSystem.h       ✅ (exists)
│   ├── SelectionSystem.h       ❌ (ADD)
│   ├── HistorySystem.h         ❌ (ADD)
│   └── System.h                ✅ (exists)
├── Editor/
│   ├── EditorCameraController.h ✅ (exists)
│   ├── EditorUI.h              ❌ (ADD)
│   ├── Panels/
│   │   ├── Panel.h             ❌ (ADD)
│   │   ├── ViewportPanel.h     ❌ (ADD)
│   │   ├── SceneHierarchyPanel.h ❌ (ADD)
│   │   └── PropertiesPanel.h   ❌ (ADD)
│   └── Gizmos/
│       ├── GizmoRenderer.h     ❌ (ADD)
│       ├── Gizmo.h             ❌ (ADD)
│       └── MovementGizmo.h     ❌ (ADD)
└── Platform/Jobs/
    ├── TaskQueue.h             ❌ (ADD)
    ├── Task.h                  ❌ (ADD)
    └── TaskPool.h              ❌ (ADD)
```
