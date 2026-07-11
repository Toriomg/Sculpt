# SelectionSystem Implementation Plan

## Architecture Overview

### Current State Analysis
- **ECS Foundation**: EnTT-based with Scene owning registry, Systems derive from abstract System base class
- **Rendering Pipeline**: Static Renderer API with BeginScene/EndScene, per-frame shader state management
- **Picking Infrastructure**: Picking.shader exists (outputs objectID + primitiveID + gl_PrimitiveID), Framebuffer class available for FBO management
- **Event System**: EventDispatcher pattern with Mouse events propagated through EditorLayer
- **Update Cycle**: EditorLayer::OnUpdate → Scene::OnUpdate → System::OnUpdate(deltaTime)

---

## Design Decisions

### 1. Picking Pass Placement
**Decision: Execute BEFORE main rendering, separate framebuffer**

**Rationale:**
- Picking framebuffer doesn't need color output, only ID data
- Executing before main render avoids viewport conflicts and is more cache-friendly
- Clear separation of concerns: identify → then highlight
- Sync approach (not async) due to immediate mouse feedback requirement

**Lifecycle Flow:**
```
EditorLayer::OnUpdate
  ├─ Scene::OnUpdate (deltaTime)
  │  ├─ PickingSystem::OnUpdate  [if mouse clicked: render picking pass]
  │  ├─ RenderingSystem::OnUpdate [render main scene + highlights]
  │  └─ SelectionHighlightSystem::OnUpdate [optional: separate highlight pass]
  └─ EditorLayer event handling [process mouse events]
```

### 2. Selection State Storage
**Decision: Dual-layer approach using SelectionComponent + SelectionContext**

**SelectionComponent** (per-entity):
- Marks entities as selectable
- Stores selection metadata (selection group, custom data)
- Optional outline/highlight properties

**SelectionContext** (system singleton):
- Central tracking of current selection (set of Entity IDs)
- Multi-selection state (shift modifier tracking)
- Selection changed signals/callbacks for UI

**Rationale:**
- Separates "can be selected" property from "is currently selected" state
- Allows reactive updates (UI binds to SelectionContext)
- Enables queries like "all selected entities" efficiently via EnTT view
- Supports frame persistence automatically

### 3. Highlight Rendering
**Decision: Shader variants + render queue reordering**

**Strategy:**
- Extend Material/Shader system with highlight variant shaders
- Main rendering pass submits all objects (selected ones last with highlight shader)
- Outline rendering via second depth-disabled pass for selected objects only

**Shader Variant Pattern:**
```
modelmesh.shader (current)
modelmesh_selected.shader (with outline/glow)
```

**Rationale:**
- Minimal performance overhead (batch-friendly)
- No additional render pass for selected objects during main rendering
- Follows existing shader pattern in codebase

### 4. Gizmo Lifecycle
**Decision: Separate GizmoRenderer system (out of scope but planned)**

**Rationale:**
- Keeps SelectionSystem focused on selection mechanics
- Gizmos have orthogonal concerns (transform manipulation)
- Can be added later without refactoring SelectionSystem

### 5. Thread Safety
**Decision: Single-threaded, sync picking**

**Rationale:**
- Main render thread already single-threaded (GLFW, OpenGL)
- Mouse input events handled synchronously in layer
- No async gains without full engine thread pool infrastructure
- Frame-latency acceptable for UI selection

### 6. Multi-Selection
**Decision: Shift+click modifier tracking at EditorLayer level**

**Implementation:**
- EditorLayer tracks shift key state from KeyboardEvents
- Passes modifier flag to SelectionSystem::OnMouseClick()
- SelectionContext handles additive vs. replace logic

---

## Data Structures

### 1. SelectionComponent (Component.h)
```cpp
struct SelectionComponent : public Component {
    uint32_t SelectionGroup = 0;  // Grouping for batch operations
    uint32_t CustomData = 0;       // User-defined selection metadata
    bool AllowsOutline = true;     // Enable outline rendering
    
    SelectionComponent() = default;
    SelectionComponent(uint32_t group) : SelectionGroup(group) {}
};
```

### 2. PickingResult (PickingSystem.h)
```cpp
struct PickingResult {
    Entity SelectedEntity = NULL_ENTITY;
    uint32_t PrimitiveID = 0xFFFFFFFF;
    Vec3 WorldPosition = Vec3(0.0f);  // From picking shader
    bool Valid = false;
};
```

### 3. SelectionContext (SelectionSystem.h)
```cpp
class SelectionContext {
public:
    // Query current selection
    const std::unordered_set<entt::entity>& GetSelectedEntities() const;
    bool IsEntitySelected(Entity entity) const;
    size_t GetSelectionCount() const;
    
    // Modify selection
    void Select(Entity entity, bool additive = false);
    void Deselect(Entity entity);
    void ClearSelection();
    void SelectMultiple(const std::vector<Entity>& entities, bool additive = false);
    
    // Signals for UI binding
    std::function<void(const std::vector<Entity>&)> OnSelectionChanged;
    
private:
    std::unordered_set<entt::entity> m_SelectedEntities;
};
```

### 4. PickingTexture (Platform/Graphics/PickingTexture.h)
```cpp
class PickingTexture {
public:
    PickingTexture(uint32_t width, uint32_t height);
    ~PickingTexture();
    
    void Bind();
    void Unbind();
    void Resize(uint32_t width, uint32_t height);
    
    // Read picking data at screen coordinates
    PickingResult ReadPixel(uint32_t x, uint32_t y) const;
    
private:
    void Invalidate();
    
    uint32_t m_FramebufferID = 0;
    uint32_t m_IDTextureID = 0;      // GL_RG32UI (objectID + primitiveID)
    uint32_t m_WorldPosTextureID = 0; // GL_RGBA32F (world position)
    uint32_t m_DepthTextureID = 0;
    uint32_t m_Width, m_Height;
};
```

### 5. PickingSystem (Core/Systems/PickingSystem.h)
```cpp
class PickingSystem : public System {
public:
    PickingSystem();
    ~PickingSystem();
    
    virtual void OnUpdate(float deltaTime) override;
    
    // Set flag for next frame's picking pass
    void RequestPickingPass(uint32_t screenX, uint32_t screenY);
    
    // Get last picking result
    const PickingResult& GetLastResult() const;
    
private:
    void ExecutePickingPass();
    void RenderPickingPass(const Camera& camera);
    
    std::unique_ptr<PickingTexture> m_PickingTexture;
    std::unique_ptr<Shader> m_PickingShader;
    PickingResult m_LastResult;
    
    bool m_PickingRequested = false;
    uint32_t m_PickX = 0, m_PickY = 0;
    
    // Window dimensions (updated on resize)
    uint32_t m_ViewportWidth = 1280;
    uint32_t m_ViewportHeight = 720;
};
```

### 6. SelectionSystem (Core/Systems/SelectionSystem.h)
```cpp
class SelectionSystem : public System {
public:
    SelectionSystem();
    ~SelectionSystem();
    
    virtual void OnAttach(Scene* scene) override;
    virtual void OnUpdate(float deltaTime) override;
    
    // Handle mouse click events
    void OnMouseClick(uint32_t screenX, uint32_t screenY, bool additive = false);
    
    // Direct API
    SelectionContext& GetSelectionContext() { return m_SelectionContext; }
    const SelectionContext& GetSelectionContext() const { return m_SelectionContext; }
    
private:
    void UpdateEntitySelectionState();
    void ApplyHighlightShaders();
    
    SelectionContext m_SelectionContext;
    PickingSystem* m_PickingSystem = nullptr;  // Non-owning reference
};
```

---

## System Lifecycle

### 1. Scene Construction (Scene::Scene)
```cpp
Scene::Scene() {
    m_Systems.emplace_back(std::make_unique<PickingSystem>());
    m_Systems.emplace_back(std::make_unique<SelectionSystem>());
    m_Systems.emplace_back(std::make_unique<RenderingSystem>());
    
    for (const auto& system : m_Systems) {
        system->OnAttach(this);
    }
}
```

Order matters: Picking first (so data available), Selection second, Rendering last.

### 2. PickingSystem::OnUpdate
```cpp
void PickingSystem::OnUpdate(float deltaTime) {
    if (!m_PickingRequested) return;
    
    if (!m_Scene) return;
    
    // Get camera for projection
    auto view = m_Scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
    Camera* camera = nullptr;
    for (auto entity : view) {
        auto& cam = view.get<CameraComponent>(entity);
        if (cam.IsPrimary) {
            camera = &cam.SceneCamera;
            break;
        }
    }
    
    if (!camera) return;
    
    // Render picking pass
    RenderPickingPass(*camera);
    
    // Read pixel data
    m_LastResult = m_PickingTexture->ReadPixel(m_PickX, m_PickY);
    m_PickingRequested = false;
}
```

### 3. SelectionSystem::OnUpdate
```cpp
void SelectionSystem::OnUpdate(float deltaTime) {
    if (!m_Scene || !m_PickingSystem) return;
    
    // Check if picking result is available
    const auto& pickResult = m_PickingSystem->GetLastResult();
    if (pickResult.Valid) {
        OnMouseClick(0, 0, false); // Process the picking result
    }
    
    // Apply highlight shaders to selected entities
    ApplyHighlightShaders();
}
```

### 4. RenderingSystem::OnUpdate (modified to handle selection)
```cpp
void RenderingSystem::OnUpdate(float deltaTime) {
    // ... existing camera/scene setup ...
    
    auto group = m_Scene->GetAllEntitiesWith<TransformComponent, MeshComponent>();
    
    // Render non-selected objects first
    for (auto entity : group) {
        if (m_Scene->HasComponent<SelectionComponent>(entity)) {
            auto& sel = m_Scene->GetComponent<SelectionComponent>(entity);
            if (selectionSystem->GetSelectionContext().IsEntitySelected(entity)) {
                continue; // Render selected objects last
            }
        }
        // ... render object ...
    }
    
    // Render selected objects with highlight shader
    for (auto entity : group) {
        if (m_Scene->HasComponent<SelectionComponent>(entity)) {
            if (selectionSystem->GetSelectionContext().IsEntitySelected(entity)) {
                // Use highlight variant material
                // ... render with outline shader ...
            }
        }
    }
    
    Renderer::EndScene();
}
```

---

## Mouse Event Integration

### EditorLayer Event Handling
```cpp
bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
    if (e.GetMouseButton() != GLFW_MOUSE_BUTTON_LEFT) return false;
    
    auto& input = Input::Get();
    double mouseX = input.GetMouseX();
    double mouseY = input.GetMouseY();
    
    // Check if Shift is held for additive selection
    bool isShiftHeld = Input::IsKeyPressed(Key::LeftShift);
    
    // Request picking and selection
    auto selSystem = m_ActiveScene->GetSystem<SelectionSystem>();
    selSystem->OnMouseClick((uint32_t)mouseX, (uint32_t)mouseY, isShiftHeld);
    
    return true; // Consume event
}
```

---

## Picking Pass Implementation

### PickingSystem::RenderPickingPass Implementation
```cpp
void PickingSystem::RenderPickingPass(const Camera& camera) {
    m_PickingTexture->Bind();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_PickingShader->Bind();
    m_PickingShader->SetUniformMat4f("u_ViewProjection", 
                                      camera.GetViewProjectionMatrix());
    
    // Render all selectable entities
    auto group = m_Scene->GetAllEntitiesWith<TransformComponent, 
                                              MeshComponent, 
                                              SelectionComponent>();
    
    for (auto entity : group) {
        auto& transform = group.get<TransformComponent>(entity);
        auto& mesh = group.get<MeshComponent>(entity);
        
        m_PickingShader->SetUniform1ui("objectID", (uint32_t)entity);
        m_PickingShader->SetUniformMat4f("u_Model", transform.Transform);
        
        // Draw mesh without material (just geometry)
        auto va = mesh.MeshAsset->GetVertexArray();
        auto ib = mesh.MeshAsset->GetIndexBuffer();
        va->Bind();
        ib->Bind();
        glDrawElements(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT, nullptr);
    }
    
    m_PickingTexture->Unbind();
}
```

### PickingTexture::ReadPixel Implementation
```cpp
PickingResult PickingTexture::ReadPixel(uint32_t x, uint32_t y) const {
    PickingResult result;
    
    // Flip Y for OpenGL coordinate system
    y = m_Height - y;
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FramebufferID);
    
    // Read object ID and primitive ID
    uint32_t idData[2];
    glReadPixels(x, y, 1, 1, GL_RG_INTEGER, GL_UNSIGNED_INT, idData);
    
    // Read world position
    float worldPos[4];
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, worldPos);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    
    result.SelectedEntity = (entt::entity)idData[0];
    result.PrimitiveID = idData[1];
    result.WorldPosition = Vec3(worldPos[0], worldPos[1], worldPos[2]);
    result.Valid = idData[0] != 0;  // 0 = empty space
    
    return result;
}
```

---

## Selection State Management

### SelectionContext Implementation
```cpp
void SelectionContext::Select(Entity entity, bool additive) {
    if (!additive) {
        m_SelectedEntities.clear();
    }
    
    bool wasAdded = m_SelectedEntities.insert(entity).second;
    if (wasAdded && OnSelectionChanged) {
        std::vector<Entity> selected(m_SelectedEntities.begin(), 
                                      m_SelectedEntities.end());
        OnSelectionChanged(selected);
    }
}

void SelectionContext::ClearSelection() {
    if (m_SelectedEntities.empty()) return;
    
    m_SelectedEntities.clear();
    if (OnSelectionChanged) {
        OnSelectionChanged({});
    }
}
```

### SelectionSystem Click Handler
```cpp
void SelectionSystem::OnMouseClick(uint32_t screenX, uint32_t screenY, 
                                    bool additive) {
    if (!m_PickingSystem) return;
    
    const auto& result = m_PickingSystem->GetLastResult();
    
    if (result.Valid) {
        // Entity selected
        m_SelectionContext.Select(result.SelectedEntity, additive);
    } else {
        // Empty space clicked - clear selection if not additive
        if (!additive) {
            m_SelectionContext.ClearSelection();
        }
    }
}
```

---

## Shader Integration

### Picking Shader (res/shaders/Picking.shader)
Already exists - uses:
- `uniform uint objectID` - entity ID
- `layout (location = 0) out uvec3 out_IDs` - objectID, gl_PrimitiveID, 0
- `layout (location = 1) out vec4 out_WorldPos` - world position

### Highlight Shader (res/shaders/modelmesh_selected.shader - NEW)
```glsl
#shader vertex
// [Same as modelmesh.shader vertex]

#shader fragment
#version 330 core

in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_TexCoord;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec4 u_OutlineColor = vec4(1.0, 0.8, 0.0, 1.0);

void main() {
    // Regular rendering with slight brightening
    vec3 texColor = texture(u_Texture, v_TexCoord).rgb;
    vec3 brightened = texColor * 1.3;  // Brighten selected objects
    
    FragColor = vec4(brightened, 1.0);
    // Optional: outline via normal-facing edges
}
```

---

## Performance Considerations

### 1. Picking Pass Optimization
- **Selective Rendering**: Only render entities with SelectionComponent
- **Simplified Shaders**: Picking shader has minimal computation (no lighting)
- **Framebuffer Reuse**: Small FBO (matching viewport) avoids memory waste
- **Lazy Evaluation**: Only run picking pass when mouse clicked (not every frame)

### 2. Highlight Rendering
- **Batch Friendly**: Use shader variants (single draw call per object)
- **No Extra Pass**: Highlight is standard render pass alternative, not additive
- **GPU Memory**: One extra material per object (negligible overhead)

### 3. Selection Queries
- **View Caching**: Use EnTT view for all selected entities
- **Set Operations**: std::unordered_set for O(1) lookup
- **Signal Overhead**: Only emit OnSelectionChanged when selection actually changes

---

## C++23 Features Applied

### 1. Concepts (Compiler-time Selection)
```cpp
template<typename Component>
concept Selectable = requires(Component c) {
    { c.SelectionGroup } -> std::convertible_to<uint32_t>;
};
```

### 2. std::expected for Error Handling
```cpp
std::expected<PickingResult, std::string> 
PickingTexture::ReadPixel(uint32_t x, uint32_t y) const {
    // Bounds checking
    if (x >= m_Width || y >= m_Height) {
        return std::unexpected("Pixel out of bounds");
    }
    // ... actual read ...
}
```

### 3. Ranges and Views
```cpp
auto selectedView = m_Scene->GetAllEntitiesWith<SelectionComponent>()
    | std::views::filter([&](Entity e) {
        return m_SelectionContext.IsEntitySelected(e);
    });
```

### 4. Lambda with Concepts
```cpp
auto highlightRenderer = [this](const Selectable auto& comp) {
    // Apply highlight shader based on selection component
};
```

---

## Dependency Graph

```
SelectionSystem
  ├─ depends: PickingSystem (non-owning ref)
  ├─ depends: Scene (registry access)
  └─ owns: SelectionContext

PickingSystem
  ├─ owns: PickingTexture
  ├─ owns: Shader (Picking.shader)
  └─ depends: Scene (registry access)

RenderingSystem (modified)
  ├─ reads: SelectionContext (via SelectionSystem)
  └─ depends: Scene, Renderer, Camera

EditorLayer
  ├─ owns: SelectionSystem, PickingSystem (via Scene)
  ├─ routes: MouseButtonPressedEvent → SelectionSystem::OnMouseClick
  └─ sync: viewport size → Picking/Rendering systems
```

---

## Implementation Sequencing

### Phase 1: Data Structures & Framebuffer
1. Create `SelectionComponent` struct in Component.h
2. Create `PickingTexture` class with FBO management
3. Create `PickingResult` struct
4. Create `SelectionContext` class (non-system utility)

### Phase 2: PickingSystem
5. Implement `PickingSystem` class with picking pass
6. Load `Picking.shader` and bind to PickingSystem
7. Implement `PickingTexture::ReadPixel()` with correct GL calls
8. Integrate viewport size tracking

### Phase 3: SelectionSystem
9. Implement `SelectionSystem` class
10. Implement selection state management methods
11. Integrate with PickingSystem reference
12. Implement highlight shader variant selection

### Phase 4: Integration
13. Update `Scene::Scene()` to instantiate both systems in order
14. Add `Scene::GetSystem<T>()` accessor
15. Update `EditorLayer::OnMouseButtonPressed()` to trigger picking
16. Update `EditorLayer::OnWindowResize()` to sync viewport
17. Modify `RenderingSystem` to apply highlight shaders

### Phase 5: Polish & Testing
18. Create outline shader variant (res/shaders/modelmesh_selected.shader)
19. Implement gizmo integration hooks (placeholder)
20. Add signal/callback mechanism for UI binding

---

## Critical Files for Implementation

- `src/Core/Components/Component.h` - Add SelectionComponent
- `src/Core/Systems/PickingSystem.h` - New picking system
- `src/Platform/Graphics/PickingTexture.h` - New texture class for FBO
- `src/Core/Systems/SelectionSystem.h` - New selection system
- `src/Core/Scene.h` - Add GetSystem<T>() accessor
- `src/Core/Systems/RenderingSystem.h` - Modify to apply highlights
- `src/Platform/Layers/EditorLayer.h` - Wire mouse events

---

## Known Limitations & Future Work

### Current Scope
- Single-entity selection (batching deferred)
- Outline via shader brightening (not edge-detect outline)
- No primitive-level selection (triangle picking available via primitiveID)
- No selection history/undo

### Future Enhancements
1. **Batch Selection**: Circle/box selection via multi-sample picking
2. **Advanced Outlining**: Screen-space edge detection pass
3. **Primitive Selection**: UI toggle to select individual triangles
4. **Selection History**: Stack-based undo/redo
5. **Gizmo Integration**: Transform controls after selection
6. **Selection Groups**: Named groups for batch operations
7. **Custom Selection Criteria**: Concept-based filtering
