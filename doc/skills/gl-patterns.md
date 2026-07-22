# OpenGL Patterns in This Codebase

## Wrapper hierarchy
```
RenderCommand              ← raw glDraw*, glClear, glViewport   (Platform/Graphics/)
  ↑
VertexArray + VertexBuffer + IndexBuffer                        (Platform/Graphics/Buffers/)
  ↑
Mesh                       ← VAO + IBO; factory methods          (Renderer/Mesh)
  ↑
Renderer::Submit           ← binds shader+uniforms, calls DrawIndexed
```

## Mesh — creation and update
```cpp
// Built-in primitives:
auto mesh = Mesh::CreateCube();
// also: CreateSphere, CreatePyramid, CreateTorus, CreateDodecahedron, CreateIcosahedron
// From raw data:
auto mesh = Mesh::CreateMeshFromData(vertices, indices);
// Re-upload CPU edits after EditMesh ops:
mesh->UpdateData(newVertices, newIndices);
```

## Shader
Shaders live in `res/shaders/`, dual-stage (`#type vertex` / `#type fragment` in one file).
```cpp
auto shader = std::make_shared<Shader>("res/shaders/modelmesh.shader");
shader->Bind();
shader->SetUniformMat4("u_ViewProjection", vp.data_ptr());
shader->SetUniformVec3("u_Color", {r, g, b});
shader->SetUniformInt("u_Texture", slot);
shader->SetUniformFloat("u_Value", f);
```

## Texture
```cpp
auto tex = std::make_shared<Texture>("res/textures/foo.png");
tex->Bind(slot);   // binds to GL_TEXTURE0 + slot
```

## Framebuffer
```cpp
FramebufferSpec spec{ width, height };
auto fb = std::make_shared<Framebuffer>(spec);
fb->Bind();
// ... render ...
fb->Unbind();
uint32_t texId = fb->GetColorAttachmentRendererID();  // pass to ImGui::Image
```

## PickingTexture
Two-attachment FBO: `ObjectID` (entity+1 as int) and `WorldPosition` (float RGB).
```cpp
pickingTex.EnableWriting();
// render picking pass with Picking.shader, u_EntityID set per draw call
pickingTex.DisableWriting();
PickingTexture::PixelInfo info = pickingTex.ReadPixel(x, y);
// info.ObjectID  → entity = ObjectID - 1  (0 = no hit)
// info.WorldPosition → Vec3 world pos
```

## Picking shader encoding
```glsl
// Picking.shader fragment
layout(location=0) out uvec3 objectID;
layout(location=1) out vec3  worldPos;
void main() {
    objectID = uvec3(u_EntityID + 1u, 0u, 0u);
    worldPos  = v_WorldPos;
}
```
Always encode as `entityID + 1` so pixel value 0 = "no hit" unambiguously.

## Uniform Buffer Objects (for planned lighting)
```cpp
// Create once:
GLuint ubo;
glGenBuffers(1, &ubo);
glBindBuffer(GL_UNIFORM_BUFFER, ubo);
glBufferData(GL_UNIFORM_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);

// Update per frame:
glBindBuffer(GL_UNIFORM_BUFFER, ubo);
glBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, dataPtr);

// Shader side:
layout(std140, binding = N) uniform LightBlock { LightData lights[MAX_LIGHTS]; };
```

## Depth test patterns
- Normal rendering: `glDepthFunc(GL_LESS)` (default).
- Infinite grid / skybox (drawn last, must appear "behind" everything): `glDepthFunc(GL_LEQUAL)`.
- Shadow map pass: depth-only FBO, no color attachment, `GL_DEPTH_COMPONENT` format.

## Rule of Five for GPU objects
Any class that calls `glDelete*` in its destructor:
```cpp
class GPUResource {
    GLuint m_ID = 0;
public:
    ~GPUResource()                              { glDeleteBuffers(1, &m_ID); }
    GPUResource(GPUResource const&)            = delete;
    GPUResource& operator=(GPUResource const&) = delete;
    GPUResource(GPUResource&& o) noexcept      : m_ID(o.m_ID) { o.m_ID = 0; }
    GPUResource& operator=(GPUResource&& o) noexcept {
        if (this != &o) { glDeleteBuffers(1, &m_ID); m_ID = o.m_ID; o.m_ID = 0; }
        return *this;
    }
};
```

## Adding a new asset type
1. Create `AssetManager/Loader/MyLoader.hpp/.cpp` implementing `IAssetLoader`.
2. The produced type must inherit `IAsset` (`Platform/CoreUtils/IAsset.hpp`).
3. Register in `AssetManager::Init()`:
   ```cpp
   s_Data->loaders.RegisterLoader(".ext", std::make_unique<MyLoader>());
   ```
