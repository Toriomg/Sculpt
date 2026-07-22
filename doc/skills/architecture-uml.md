# Architecture — UML Class Reference

Full codebase class map with relationships. Read before any structural change.
Diagrams use [Mermaid](https://mermaid.js.org/) class diagram syntax — render in any Mermaid viewer or GitHub.

> **Notation:** `*--` composition (owner), `o--` aggregation (shared), `-->` association (reference/pointer), `<|--` inheritance.

---

## Layer map

| Layer | Directories | Rule |
|---|---|---|
| L5 | `Platform/Application` | Top of the stack |
| L4 | `Editor/` | May include L3, L2, L1 |
| L3 | `Core/`, `Renderer/`, `AssetManager/`, `Core/EditMesh/` | May include L2, L1 |
| L2 | `Platform/CoreUtils/`, `Platform/Graphics/`, `Platform/System/`, `Platform/Layers/`, `Platform/Jobs/` | May include L1 |
| L1 | `Core/glhead.hpp` | System OpenGL/GLFW headers only |

---

## Diagram 1 — Application & Layer Stack (L5/L2)

```mermaid
classDiagram
direction TB

class Application {
    -GlfwWindow m_Window
    -LayerStack m_LayerStack
    -ImGuiLayer* m_ImGuiLayer
    +Run()
    +OnEvent(Event&)
}

class LayerStack {
    -vector~unique_ptr~Layer~~ m_Layers
    +PushLayer(unique_ptr~Layer~)
    +PopLayer(Layer*)
    +begin() / end()
    +rbegin() / rend()
}

class Layer {
    <<abstract>>
    #string m_DebugName
    +OnAttach()
    +OnDetach()
    +OnUpdate(float)
    +OnEvent(Event&)
    +OnImGuiRender()
}

class EditorLayer {
    <<L4>>
    ...see Diagram 3...
}

class ImGuiLayer {
    <<L4>>
    +Begin()
    +End()
}

class ServiceLayer {
    <<L4>>
}

Application *-- LayerStack
Application *-- GlfwWindow
LayerStack *-- "0..*" Layer : owns (unique_ptr)
Layer <|-- EditorLayer
Layer <|-- ImGuiLayer
Layer <|-- ServiceLayer
```

**Event flow:** `GlfwWindow callbacks → Application::OnEvent → LayerStack::rbegin()→rend()` (reverse, last-pushed first).
**Update flow:** `LayerStack::begin()→end()` (insertion order, EditorLayer first).

---

## Diagram 2 — Scene & ECS Systems (L3 Core)

```mermaid
classDiagram
direction TB

class Scene {
    -entt_registry m_Registry
    -vector~unique_ptr~System~~ m_Systems
    +CreateGameObject(string) Entity
    +DestroyEntity(Entity)
    +AddComponent~T~(Entity, args) T&
    +GetComponent~T~(Entity) T&
    +SetComponent~T~(Entity, args) T&
    +HasComponent~T~(Entity) bool
    +RemoveComponent~T~(Entity)
    +GetAllEntitiesWith~...~() view
    +GetSystem~T~() T*
    +OnUpdate(float)
}

class System {
    <<abstract>>
    #Scene* m_Scene
    +OnAttach(Scene*)
    +OnUpdate(float)*
}

class RenderingSystem {
    -Matx4f m_GlobalTransform
    +OnUpdate(float)
    +SetGlobalTransform(Matx4f)
}

class SelectionSystem {
    -SelectionContext m_SelectionContext
    -PickingSystem* m_PickingSystem
    +OnAttach(Scene*)
    +OnUpdate(float)
    +OnMouseClick(uint32 x, uint32 y, bool additive)
    +GetSelectionContext() SelectionContext&
}

class PickingSystem {
    -unique_ptr~PickingTexture~ m_PickingTexture
    -unique_ptr~Shader~ m_PickingShader
    -PickingResult m_LastResult
    -bool m_PickingRequested
    -uint32 m_PickX, m_PickY
    +OnAttach(Scene*)
    +OnUpdate(float)
    +RequestPickingPass(uint32 x, uint32 y)
    +GetLastResult() PickingResult
    +OnWindowResize(uint32 w, uint32 h)
    +SetGlobalTransform(Matx4f)
}

class HistorySystem {
    -vector~unique_ptr~Command~~ m_UndoStack
    -vector~unique_ptr~Command~~ m_RedoStack
    -k_MaxHistory = 100
    +OnUpdate(float)
    +Push(unique_ptr~Command~)
    +Undo()
    +Redo()
    +CanUndo() bool
    +CanRedo() bool
    +Clear()
}

class SelectionContext {
    -unordered_set~entt_entity~ m_SelectedEntities
    -entt_entity m_ActiveEntity
    +Select(Entity, bool additive)
    +Deselect(Entity)
    +ClearSelection()
    +SelectMultiple(vector~Entity~, bool)
    +IsEntitySelected(Entity) bool
    +GetActiveEntity() Entity
    +GetSelectionCount() size_t
    +OnSelectionChanged : function
}

class Command {
    <<abstract>>
    +Execute()*
    +Undo()*
    +GetDescription() string*
}

class MultiTransformCommand {
    -Scene* m_Scene
    -vector~pair~Entity~TransformComponent~~ m_Before
    -vector~pair~Entity~TransformComponent~~ m_After
    +Execute()
    +Undo()
    +GetDescription() string
}

class MeshEditCommand {
    -Scene* m_Scene
    -entt_entity m_Entity
    -EditModeSystem* m_EditSystem
    -vector~EditVertex~ m_Before / m_After
    -vector~uint32~ m_IndsBefore / m_IndsAfter
    -string m_Description
    +Execute()
    +Undo()
    +GetDescription() string
}

Scene *-- "1" entt_registry : owns
Scene *-- "0..*" System : owns (unique_ptr)
System <|-- RenderingSystem
System <|-- SelectionSystem
System <|-- PickingSystem
System <|-- HistorySystem
SelectionSystem *-- SelectionContext : owns
SelectionSystem --> PickingSystem : non-owning ref
PickingSystem *-- PickingTexture : owns
PickingSystem *-- Shader : owns (picking shader)
HistorySystem *-- "0..*" Command : undo/redo stacks
Command <|-- MultiTransformCommand
Command <|-- MeshEditCommand
MultiTransformCommand --> Scene : ref
MeshEditCommand --> Scene : ref
MeshEditCommand --> EditModeSystem : ref
```

**Note:** `SelectionSystem` obtains `m_PickingSystem` via `scene->GetSystem<PickingSystem>()` in `OnAttach`. `HistorySystem` is also fetched at attach time by systems that push commands.

---

## Diagram 3 — Editor Layer & UI (L4)

```mermaid
classDiagram
direction TB

class EditorLayer {
    -unique_ptr~Scene~ m_ActiveScene
    -Camera m_EditorCamera
    -Entity m_CameraEntity
    -unique_ptr~EditorCameraController~ m_CameraController
    -unique_ptr~InfGrid~ m_Grid
    -unique_ptr~Framebuffer~ m_ViewportFBO
    -bool m_InEditMode
    -unique_ptr~EditModeSystem~ m_EditModeSystem
    -unique_ptr~GizmoRenderer~ m_GizmoRenderer
    -unique_ptr~EntityFactory~ m_EntityFactory
    -unique_ptr~ViewportPanel~ m_ViewportPanel
    -unique_ptr~OutlinerPanel~ m_OutlinerPanel
    -unique_ptr~InspectorPanel~ m_InspectorPanel
    -unique_ptr~MainMenuBar~ m_MainMenuBar
    -unique_ptr~ScenePanel~ m_ScenePanel
    +OnAttach()
    +OnUpdate(float)
    +OnImGuiRender()
    +OnEvent(Event&)
}

class Panel {
    <<abstract>>
    +bool IsVisible
    +OnImGuiRender()*
}

class ViewportPanel {
    -Framebuffer* m_Framebuffer
    -ResizeCallback m_OnResize
    -Vec2 m_ViewportMin
    +IsFocused() bool
    +IsHovered() bool
    +GetViewportMin() Vec2
}

class OutlinerPanel {
    -Scene* m_Scene
    -SelectionSystem* m_SelectionSystem
    +SetInEditMode(bool, string_view)
    +TriggerDeleteConfirmation()
}

class InspectorPanel {
    -Scene* m_Scene
    -SelectionContext* m_SelectionContext
    -HistorySystem* m_HistSys
    -GizmoRenderer* m_GizmoRenderer
    -EditModeSystem* m_EditModeSystem
    -Entity m_SnapshotEntity
    -TransformComponent m_TransformSnapshot
}

class ScenePanel {
    -Scene* m_Scene
    -Camera* m_Camera
}

class MainMenuBar {
    -Scene* m_Scene
    -EntityFactory* m_EntityFactory
    -EditorLayer* m_EditorLayer
}

class GizmoRenderer {
    -Scene& m_Scene
    -SelectionContext& m_SelCtx
    -Camera& m_Camera
    -HistorySystem* m_HistSys
    -GizmoMode m_Mode
    -GizmoSpace m_Space
    -PivotMode m_PivotMode
    -SnapConfig m_Snap
    -bool m_IsDragging
    -GizmoAxis m_DragAxis
    -vector~pair~Entity~TransformComponent~~ m_SnapshotTransforms
    +Draw()
    +OnMouseButtonPressed(float vx, float vy) bool
    +OnMouseButtonReleased() bool
    +OnMouseMoved(float vx, float vy) bool
    +SetMode(GizmoMode)
    +ToggleSpace()
    +SetPivotMode(PivotMode)
    +SetSnapConfig(SnapConfig)
}

class EditorCameraController {
    -Camera* m_CameraToControl
    -Vec2 m_LastMousePosition
    -float m_MovementSpeed
    -float m_MouseSensitivity
    +OnUpdate(float)
    +OnScrolled(float)
}

class EntityFactory {
    -Scene* m_Scene
    -shared_ptr~Shader~ m_DefaultShader
    +SpawnPrimitive(PrimitiveType)
    +SpawnFromFile(string) expected~void~string~
}

class GizmoMode {
    <<enumeration>>
    Translation
    Rotation
    Scale
}

class GizmoSpace {
    <<enumeration>>
    Global
    Local
}

class PivotMode {
    <<enumeration>>
    IndividualOrigins
    MedianPoint
    ActiveElement
}

Panel <|-- ViewportPanel
Panel <|-- OutlinerPanel
Panel <|-- InspectorPanel
Panel <|-- ScenePanel
Panel <|-- MainMenuBar
EditorLayer *-- Scene
EditorLayer *-- EditorCameraController
EditorLayer *-- InfGrid
EditorLayer *-- Framebuffer
EditorLayer *-- EditModeSystem
EditorLayer *-- GizmoRenderer
EditorLayer *-- EntityFactory
EditorLayer *-- ViewportPanel
EditorLayer *-- OutlinerPanel
EditorLayer *-- InspectorPanel
EditorLayer *-- MainMenuBar
EditorLayer *-- ScenePanel
EditorCameraController --> Camera : controls (ptr)
GizmoRenderer --> Scene : ref
GizmoRenderer --> SelectionContext : ref
GizmoRenderer --> Camera : ref
GizmoRenderer --> HistorySystem : ref (via Scene::GetSystem)
InspectorPanel --> Scene : ref
InspectorPanel --> SelectionContext : ref
InspectorPanel --> GizmoRenderer : ref
InspectorPanel --> EditModeSystem : ref
OutlinerPanel --> Scene : ref
OutlinerPanel --> SelectionSystem : ref
ViewportPanel --> Framebuffer : ref (non-owning)
EntityFactory --> Scene : ref
```

---

## Diagram 4 — Edit Mode (L3)

```mermaid
classDiagram
direction TB

class EditModeSystem {
    -Scene* m_Scene
    -Camera& m_Camera
    -EditMesh m_EditMesh
    -Matx4f m_GlobalTransform
    -optional~ExtrudeState~ m_ExtrudeState
    -shared_ptr~VertexArray~ m_SelectionVAO
    -shared_ptr~VertexBuffer~ m_SelectionVBO
    -uint32 m_SelectionDrawCount
    -uint32 m_SelectionPrimitive
    +Enter(entt_entity)
    +Exit()
    +IsActive() bool
    +GetEditedEntity() entt_entity
    +GetEditMesh() EditMesh const&
    +SetElementMode(ElementMode)
    +SetViewportSize(uint32, uint32)
    +SetGlobalTransform(Matx4f)
    +OnMouseClick(uint32 primitiveID, float sx, float sy, bool additive)
    +Extrude()
    +UpdateGrab(float dx, float dy)
    +ConfirmGrab() / CancelGrab()
    +SyncFromVertices(verts, inds)
    +DrawOverlay(Matx4f)
}

class EditMesh {
    +vector~EditVertex~ vertices
    +vector~uint32~ indices
    +ElementMode mode
    +unordered_set~uint32~ selectedVertices
    +unordered_set~uint32~ selectedFaces
    +unordered_set~uint64~ selectedEdges
    +bool selectionDirty
    -entt_entity m_Entity
    +IsActive() bool
    +GetEntity() entt_entity
    +FaceCount() uint32
    +PackEdge(a, b) uint64$
    +ClearSelection()
    +SetEntity(entt_entity)
    +Clear()
}

class EditVertex {
    +Vec3 position
    +Vec3 normal
    +Vec2 texCoord
    +bool selected
}

class ElementMode {
    <<enumeration>>
    Vertex
    Edge
    Face
}

class ExtrudeState {
    +size_t vertexCountBefore
    +vector~uint32~ indicesBefore
    +unordered_set selectedFaces/Verts/Edges
    +vector~uint32~ grabbedVerts
    +vector~Vec3~ basePositions
    +Vec3 normal
    +float offset
    +vector~EditVertex~ verticesBefore
    +vector~pair~ wallTopMirrors
}

EditModeSystem *-- EditMesh
EditModeSystem *-- ExtrudeState : optional
EditMesh *-- "0..*" EditVertex
EditMesh --> ElementMode
EditModeSystem --> Scene : ref
EditModeSystem --> Camera : const ref
```

**Picking in edit mode:**
- Enter: `PickingSystem` renders `primitiveID` per triangle (encodes `gl_PrimitiveID`).
- Click: `EditModeSystem::OnMouseClick(primitiveID, sx, sy, additive)` resolves triangle → vertex/edge/face based on `m_EditMesh.mode`.
- Edge key: `PackEdge(a, b)` = `(min<<32) | max` — order-independent.

---

## Diagram 5 — Renderer & Assets (L3)

```mermaid
classDiagram
direction TB

class Renderer {
    <<static>>
    +Init()$
    +BeginScene(Matx4f viewProj)$
    +Submit(Mesh, Material, Matx4f)$
    +SubmitWireframe(Mesh, Matx4f)$
    +SubmitFlat(Mesh, Vec4, Matx4f)$
    +SubmitOutline(Mesh, Vec4, float, Matx4f)$
    +SubmitEditOverlay(Mesh, Matx4f)$
    +SubmitSelectionHighlight(VertexArray, uint32, uint32, Matx4f)$
    +EndScene()$
    +SetSceneClearColor(Vec4)$
    +SetGridColor(Vec3)$
    +OnWindowResize(uint32, uint32)$
}

class Camera {
    -Matx4f m_ProjectionMatrix
    -Matx4f m_ViewMatrix
    -Matx4f m_ViewProjectionMatrix
    -Vec3 m_Position
    -Vec3 m_Front, m_Up, m_Right
    -float m_Pitch, m_Yaw
    -ProjectionType m_ProjectionType
    -float m_PerspectiveFOV
    -float m_OrthographicSize
    -float m_NearClip, m_FarClip
    +SetProjection(ProjectionType)
    +SetViewportSize(float, float)
    +GetViewProjectionMatrix() Matx4f
    +GetPosition() Vec3
    +SetYaw(float) / SetPitch(float)
    +GetFrontDirection() Vec3
}

class Mesh {
    +AssetHandle Handle
    -shared_ptr~VertexArray~ m_VertexArray
    -shared_ptr~IndexBuffer~ m_IndexBuffer
    -shared_ptr~VertexBuffer~ m_VertexBuffer
    -vector~Vertex~ m_CpuVertices
    -vector~uint32~ m_CpuIndices
    -uint32 m_VertexCount
    +GetVertexArray() shared_ptr~VertexArray~
    +GetCpuVertices() vector~Vertex~
    +GetCpuIndices() vector~uint32~
    +UpdateData(verts, inds)
    +UpdateVertices(verts)
    +CreateCube(float)$
    +CreateSphere(float, int, int)$
    +CreatePyramid(float)$
    +CreateTorus(...)$
    +CreateDodecahedron(float)$
    +CreateIcosahedron(float)$
    +CreateArrow(int)$
    +CreateCone(int)$
    +CreateMeshFromData(...)$
}

class Material {
    -shared_ptr~Shader~ m_Shader
    -AssetHandle m_TextureHandle
    +GetShader() shared_ptr~Shader~
    +GetTextureHandle() AssetHandle
    +SetTexture(AssetHandle)
}

class InfGrid {
    -unique_ptr~Shader~ m_Shader
    -uint m_VAO
    +Draw(view, projection, cameraPos)
}

class AssetManager {
    <<static>>
    +Init()$ / Shutdown()$
    +Load(string) AssetHandle$
    +LoadAsync(string, callback)$
    +Get(AssetHandle) shared_ptr~void~$
    +GetAs~T~(AssetHandle) shared_ptr~T~$
}

class AssetHandle {
    +uint64_t ID
    +operator bool()
    +Create()$
}

class Vertex {
    +Vec3 pos
    +Vec3 normal
    +Vec2 texCoord
}

Renderer --> RenderCommand : delegates raw GL calls
Renderer --> Mesh : draws via
Renderer --> Material : binds
Material *-- Shader : shared ownership
Material --> AssetHandle : texture ref
Mesh *-- VertexArray
Mesh *-- IndexBuffer
Mesh *-- VertexBuffer
Mesh *-- "0..*" Vertex : CPU copy (for edit mode)
AssetManager --> AssetHandle : produces/resolves
InfGrid *-- Shader
```

---

## Diagram 6 — Platform Graphics (L2)

```mermaid
classDiagram
direction LR

class Shader {
    -uint m_RendererID
    -string m_FilePath
    -map~string~GLint~ m_UniformCache
    +Bind() / Unbind()$
    +SetUniform1i(name, int)
    +SetUniform1f(name, float)
    +SetUniform3f(name, f, f, f)
    +SetUniform4f(name, f, f, f, f)
    +SetUniformMat4f(name, Matx4f)
}

class Texture {
    +AssetHandle Handle
    -uint m_RendererID
    -TextureSpecification m_Specification
    +Bind(uint slot)
    +Unbind()$
    +GetRendererID() uint
}

class Framebuffer {
    -uint m_RendererID
    -uint m_ColorAttachmentID
    -uint m_DepthAttachmentID
    -uint m_Width, m_Height
    +Bind() / Unbind()$
    +Resize(uint, uint)
    +GetColorAttachmentRendererID() uint
}

class PickingTexture {
    -uint m_FramebufferID
    -uint m_IDTextureID
    -uint m_WorldPosTextureID
    -uint m_DepthTextureID
    -uint m_Width, m_Height
    +Bind() / Unbind()$
    +Resize(uint, uint)
    +ReadPixel(uint x, uint y) PickingResult
}

class PickingResult {
    +uint32 ObjectID
    +uint32 PrimitiveID
    +Vec3 WorldPosition
    +bool Valid
}

class RenderCommand {
    <<static>>
    +Init()$
    +SetClearColor(Vec4)$
    +Clear()$
    +SetDepthFunc(DepthFunc)$
    +Draw(VertexArray, IndexBuffer)$
    +DrawPoints(VertexArray, uint)$
    +DrawLines(VertexArray, uint)$
    +DrawTrianglesArray(VertexArray, uint)$
    +SetViewport(x, y, w, h)$
}

class VertexArray {
    -uint m_RendererID
    +Bind() / Unbind()$
    +AddVertexBuffer(VertexBuffer)
    +SetIndexBuffer(IndexBuffer)
}

class VertexBuffer {
    -uint m_RendererID
    +Bind() / Unbind()$
    +SetData(void*, uint size)
}

class IndexBuffer {
    -uint m_RendererID
    -uint m_Count
    +Bind() / Unbind()$
    +GetCount() uint
}

PickingTexture --> PickingResult : produces
RenderCommand --> VertexArray : uses
RenderCommand --> IndexBuffer : uses
VertexArray *-- VertexBuffer
VertexArray *-- IndexBuffer
```

---

## ECS Component Map

All components live in `Core/Components/Component.hpp`. They are plain data — no methods (except `TransformComponent::GetMatrix`), no virtuals.

```mermaid
classDiagram
direction LR

class NameComponent {
    +string Name
}

class TransformComponent {
    +Vec3 Translation
    +Vec3 EulerDegrees
    +Vec3 Scale
    +GetMatrix() Matx4f
}

class MeshComponent {
    +shared_ptr~Mesh~ MeshAsset
    +shared_ptr~Material~ MaterialAsset
    +bool Wireframe
}

class CameraComponent {
    +Camera SceneCamera
    +bool IsPrimary
}

class SelectionComponent {
    +uint32 SelectionGroup
    +uint32 CustomData
    +bool AllowsOutline
}

note for MeshComponent "Mesh and Material are forward-declared in the header.\nFull includes only in .cpp files."
note for CameraComponent "Camera stored by value — exception to AssetHandle rule.\nCamera has no Core/ dependencies."
```

---

## Key Cross-System Pointer Map

Non-obvious references between systems and classes:

| Holder | Field | Target | How obtained |
|---|---|---|---|
| `SelectionSystem` | `m_PickingSystem` | `PickingSystem*` | `scene->GetSystem<PickingSystem>()` in `OnAttach` |
| `GizmoRenderer` | `m_HistSys` | `HistorySystem*` | `scene.GetSystem<HistorySystem>()` in `OnAttach` |
| `InspectorPanel` | `m_HistSys` | `HistorySystem*` | passed from `EditorLayer` after scene init |
| `InspectorPanel` | `m_EditModeSystem` | `EditModeSystem*` | `SetEditModeSystem()` called from `EditorLayer::OnAttach` |
| `MeshEditCommand` | `m_EditSystem` | `EditModeSystem*` | passed at command construction site |
| `EditorCameraController` | `m_CameraToControl` | `Camera*` | pointer to `EditorLayer::m_EditorCamera` |
| `OutlinerPanel` | `m_SelectionSystem` | `SelectionSystem*` | `scene->GetSystem<SelectionSystem>()` |
| `ViewportPanel` | `m_Framebuffer` | `Framebuffer*` | pointer to `EditorLayer::m_ViewportFBO` |

---

## Planned Extensions

Classes and relationships **not yet implemented**. Each block is a self-contained implementation unit. Refer to `doc/skills/planned-arch.md` for file locations and full specs.

### A — Lighting System
```mermaid
classDiagram
direction TB

class LightingSystem {
    <<planned - Core/Systems/ - L3>>
    -LightUBO m_UBO
    +OnUpdate(float)
    -CollectLights()
    -TriggerShadowPasses()
}

class ShadowMap {
    <<planned - Platform/Graphics/ - L2>>
    -uint m_FBO
    -uint m_DepthTexture
    -uint m_Width, m_Height
    +Bind() / Unbind()$
    +GetDepthTextureID() uint
}

class PointLightComponent {
    <<planned - Component.hpp>>
    +Vec3 Color
    +float Intensity
    +float Radius
}

class DirectionalLightComponent {
    <<planned - Component.hpp>>
    +Vec3 Color
    +float Intensity
    +bool CastShadows
}

class SpotLightComponent {
    <<planned - Component.hpp>>
    +Vec3 Color
    +float Intensity
    +float Radius
    +float InnerAngle
    +float OuterAngle
    +bool CastShadows
}

class VisibilityComponent {
    <<planned - Component.hpp>>
    +bool Visible
}

LightingSystem *-- ShadowMap : one per shadow-casting light
LightingSystem --> PointLightComponent : collects
LightingSystem --> DirectionalLightComponent : collects
LightingSystem --> SpotLightComponent : collects
```

### B — Modifier Stack
```mermaid
classDiagram
direction TB

class IModifier {
    <<planned - Core/Modifiers/ - L3>>
    <<abstract>>
    +Apply(EditMesh&)*
    +Name() string*
    +bool Enabled
}

class SubdivideModifier {
    +int Level
    +Apply(EditMesh&)
    +Name() string
}

class MirrorModifier {
    +bool AxisX, AxisY, AxisZ
    +float MergeThreshold
    +bool Clipping
    +Apply(EditMesh&)
}

class ArrayModifier {
    +int Count
    +OffsetMode mode
    +Vec3 FixedOffset
    +Apply(EditMesh&)
}

class SolidifyModifier {
    +float Thickness
    +float Offset
    +bool FillRim
    +Apply(EditMesh&)
}

class SmoothModifier {
    +int Iterations
    +float Factor
    +Apply(EditMesh&)
}

IModifier <|-- SubdivideModifier
IModifier <|-- MirrorModifier
IModifier <|-- ArrayModifier
IModifier <|-- SolidifyModifier
IModifier <|-- SmoothModifier
```
`MeshComponent` gains `vector<unique_ptr<IModifier>> Modifiers` + `bool ModifiersDirty`. `RenderingSystem` evaluates the stack when dirty and re-uploads — does not modify the base mesh.

### C — Post-Process Stack
```mermaid
classDiagram
direction TB

class PostProcessPass {
    <<planned - Renderer/PostProcess/ - L3>>
    <<abstract>>
    +Apply(Framebuffer& src, Framebuffer& dst)*
}

class SSAOPass {
    +Apply(Framebuffer&, Framebuffer&)
}

class BloomPass {
    +float Threshold
    +float Intensity
    +Apply(Framebuffer&, Framebuffer&)
}

class FXAAPass {
    +Apply(Framebuffer&, Framebuffer&)
}

PostProcessPass <|-- SSAOPass
PostProcessPass <|-- BloomPass
PostProcessPass <|-- FXAAPass
```
`EditorLayer` gains `vector<unique_ptr<PostProcessPass>> m_PostProcessPasses`. Evaluated after main render, before viewport FBO; ping-pongs two intermediate `Framebuffer` objects.

### D — Scene Serialization
```mermaid
classDiagram
direction LR

class SceneSerializer {
    <<planned - Editor/ - L4>>
    +Save(Scene&, path)$
    +Load(path) unique_ptr~Scene~$
}
```
JSON format. Asset paths stored relative to project root. Re-loads via `AssetManager::Load()`. L4 because it knows the full component set.

### E — Preferences & Keymap
```mermaid
classDiagram
direction LR

class Preferences {
    <<planned - Editor/ - L4>>
    +float snapTranslate, snapRotate, snapScale
    +int autosaveIntervalMin
    +int undoDepth
    +int shadowMapRes
    +float fontSize
    +int themeIndex
    +KeymapRegistry keymap
    +Load(path)$
    +Save(path)$
}

class KeymapRegistry {
    <<planned>>
    -flat_map~ActionID~KeyBinding~ m_Map
    +IsPressed(ActionID, Event) bool
    +Rebind(ActionID, KeyBinding)
    +ResetToDefaults()
}

class PreferencesPanel {
    <<planned - Editor/Panels/ - L4>>
    -Preferences* m_Prefs
    +OnImGuiRender()
}

Preferences *-- KeymapRegistry
PreferencesPanel --> Preferences : ref
```

### F — Parent/Child Hierarchy
```mermaid
classDiagram

class ParentComponent {
    <<planned - Component.hpp>>
    +entt_entity Parent
}
```
`TransformComponent` becomes local-space. `RenderingSystem` DFS-traverses entity graph each frame to compute world transforms. `Scene` gains `SetParent(child, parent)` / `ClearParent(child)`.

### G — PBR Material Expansion
```mermaid
classDiagram

class Material {
    <<expanded - Renderer/Material.hpp>>
    +shared_ptr~Shader~ Shader
    +Vec3 Albedo
    +float Metallic
    +float Roughness
    +float EmissiveStr
    +AssetHandle AlbedoMap
    +AssetHandle NormalMap
    +AssetHandle MetallicRoughnessMap
    +AssetHandle AOMap
    +AssetHandle EmissiveMap
}
```
`MeshComponent::MaterialAsset` → `vector<shared_ptr<Material>>` (multiple material slots). `RenderingSystem` issues one draw call per slot.

---

## Full Ownership Tree (text)

```
Application
├── GlfwWindow
└── LayerStack
    ├── ImGuiLayer
    ├── ServiceLayer
    └── EditorLayer
        ├── Scene
        │   ├── entt::registry  (all entities + components)
        │   ├── RenderingSystem
        │   ├── SelectionSystem
        │   │   └── SelectionContext
        │   ├── PickingSystem
        │   │   ├── PickingTexture (FBO + 2 attachments)
        │   │   └── Shader (picking shader)
        │   └── HistorySystem
        │       ├── [undo stack] Command*
        │       └── [redo stack] Command*
        ├── Camera (EditorCamera — value, not owned via ptr)
        ├── EditorCameraController → Camera*
        ├── InfGrid
        │   └── Shader
        ├── Framebuffer (viewport FBO)
        ├── EditModeSystem → Scene*, Camera&
        │   ├── EditMesh
        │   │   └── vector<EditVertex>
        │   └── VertexArray/VertexBuffer (selection overlay)
        ├── GizmoRenderer → Scene&, SelectionContext&, Camera&, HistorySystem*
        │   └── Mesh (arrow, ring, cone, cube — gizmo shapes)
        ├── EntityFactory → Scene*
        │   └── Shader (default shader)
        ├── ViewportPanel → Framebuffer*
        ├── OutlinerPanel → Scene*, SelectionSystem*
        ├── InspectorPanel → Scene*, SelectionContext*, GizmoRenderer*, EditModeSystem*
        ├── ScenePanel
        └── MainMenuBar

Entities (in entt::registry):
└── [entity]
    ├── NameComponent
    ├── TransformComponent
    ├── MeshComponent → shared_ptr<Mesh>, shared_ptr<Material>
    ├── CameraComponent (Camera by value)
    └── SelectionComponent

Mesh
├── VertexArray
│   ├── VertexBuffer
│   └── IndexBuffer
└── vector<Vertex>  (CPU copy for edit mode)

Material
└── shared_ptr<Shader>
    (+ AssetHandle → Texture resolved at draw time via AssetManager)
```
