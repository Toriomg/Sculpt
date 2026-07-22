# Planned Architecture
Not yet implemented. Place files as shown; follow layer rules.

## Gizmo extensions (`Editor/Gizmos/` — L4, no new files)
- `Gizmo.hpp` gains `GizmoMode::Scale` and `GizmoSpace::Global / Local / Normal`.
- `GizmoRenderer` gains `m_Space` field and `m_SnapConfig` (increment per axis, Ctrl toggle).

## Modifier stack (`Core/Modifiers/` — L3)
```
IModifier.hpp        pure interface: Apply(EditMesh&) = 0, Name() → string
SubdivideModifier    Catmull-Clark; level 0–6; per-edge crease weight
MirrorModifier       axis flags; merge threshold; clipping toggle
ArrayModifier        count + offset mode (fixed / relative / object)
SolidifyModifier     thickness; offset −1→1; Fill Rim toggle
SmoothModifier       iterations; factor; optional vertex weight mask
```
`MeshComponent` gains `vector<unique_ptr<IModifier>> Modifiers` + `bool ModifiersDirty`.
`RenderingSystem` evaluates stack when dirty: base `EditMesh` → `Apply()` each modifier → GPU upload. Does not modify the base mesh.

## Lighting (`Core/Systems/LightingSystem` — L3)
Add to `Core/Components/Component.hpp`:
```cpp
struct PointLightComponent       { Vec3 Color; float Intensity; float Radius; };
struct DirectionalLightComponent { Vec3 Color; float Intensity; bool CastShadows; };
struct SpotLightComponent        { Vec3 Color; float Intensity; float Radius;
                                   float InnerAngle; float OuterAngle; bool CastShadows; };
struct VisibilityComponent       { bool Visible = true; };
```
`LightingSystem` collects lights → `LightUBO` (UBO) → uploads once per frame. Triggers depth-pass for each `CastShadows` light before main render. `RenderingSystem` binds UBO before draw calls.
Shadow maps: `Platform/Graphics/ShadowMap.hpp/.cpp` (L2) — depth-only FBO, `GL_TEXTURE_2D` depth. Sample via `sampler2DShadow` + PCF in `pbr.shader`.

## Parent/child hierarchy
`struct ParentComponent { entt::entity Parent; }` in `Component.hpp`.
`TransformComponent` becomes local-space. `RenderingSystem` DFS-traverses entity graph each frame for world transforms before submitting.
`Scene` gains `SetParent(child, parent)` / `ClearParent(child)` (bakes world transform on unparent).

## Scene serialization (`Editor/SceneSerializer.hpp/.cpp` — L4)
JSON format. `Save(Scene&, path)` / `Load(path) → Scene`.
Asset paths stored relative to project root. Re-loads via `AssetManager::Load()`.
L4 (not L3) because serializer knows the full component set.

## Material expansion (`Renderer/Material.hpp`)
```cpp
struct Material : IAsset {
    std::shared_ptr<Shader> Shader;
    Vec3  Albedo = {1, 1, 1};       float Metallic    = 0.0f;
    float Roughness = 0.5f;         float EmissiveStr  = 0.0f;
    AssetHandle AlbedoMap, NormalMap, MetallicRoughnessMap, AOMap, EmissiveMap;
};
```
`MeshComponent::MaterialAsset` → `vector<shared_ptr<Material>>`. One draw call per slot.

## Post-process stack (`Renderer/PostProcess/` — L3)
```
PostProcessPass.hpp    interface: Apply(Framebuffer& src, Framebuffer& dst) = 0
SSAOPass               G-buffer normals+depth → hemisphere sampling → bilateral blur → AO
BloomPass              bright-pass threshold → Gaussian blur chain → additive composite
FXAAPass               single full-screen FXAA shader pass
```
`EditorLayer` owns `vector<unique_ptr<PostProcessPass>> m_PostProcessPasses`.
Ping-pongs two intermediate `Framebuffer` objects after main render, before viewport FBO.

## Skybox / HDR (`Renderer/Skybox.hpp/.cpp` — L3)
Wraps `GL_TEXTURE_CUBE_MAP`. `.hdr` equirectangular → render-to-cubemap at startup (6 faces, perspective camera).
Draw with cube + `skybox.shader`, `glDepthFunc(GL_LEQUAL)` (must be last, behind all geometry).
IBL (irradiance map + specular mip chain) deferred until PBR shader needs it.

## Preferences & Keymap (`Editor/Preferences.hpp/.cpp` — L4)
Plain struct: snap increments, autosave interval, undo depth, shadow map resolution, font size, theme index.
Serialized to `~/.config/opengl-modeler/prefs.json`.
`KeymapRegistry`: `flat_map<ActionID, KeyBinding>`. All input handlers check `keymap.IsPressed(action, event)` instead of raw GLFW key codes.

## New shaders (`res/shaders/`)
| File | Purpose |
|---|---|
| `pbr.shader` | Cook-Torrance BRDF; consumes `LightUBO`; normal map via TBN matrix |
| `shadow.shader` | Depth-only pass for shadow map generation |
| `skybox.shader` | Cubemap background; VS writes `gl_Position.z = gl_Position.w` |
| `ssao.shader` | Hemisphere sample + depth compare → AO factor |
| `bloom.shader` | Bright-pass + separable Gaussian blur |
| `fxaa.shader` | Luminance-edge detect + sub-pixel blend |
