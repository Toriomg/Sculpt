# Feature Roadmap — Quick Reference
One-line specs. Full details (algorithm, edge cases, parameters) in `doc/features.md`.

**After implementing a feature, delete its entry from this file and from `doc/features.md`.**

## Implemented
Cube/Sphere/Pyramid/Torus/Cone/Arrow/Dodecahedron/Icosahedron | GPU picking | Multi-select (Shift+click) | Translation gizmo (T) | Rotation gizmo (R) | Undo/Redo (Ctrl+Z/Y, 100 deep) | Orbit camera (MMB+scroll) | Infinite Y=0 grid | .obj import | .png/.jpg import | Outliner/Inspector/Viewport/MenuBar panels | Per-mesh wireframe | ECS (EnTT)

---

## Transform & Gizmos
| Feature | Shortcut | One-line spec |
|---|---|---|
| Scale gizmo | S (+X/Y/Z) | 3 axis handles + center cube; reads/writes `TransformComponent.Scale`; undo on mouse-release |
| Gizmo space | G | Cycles Global→Local→Normal; passes object rotation to gizmo shader and drag math |
| Snap | Ctrl+drag | Floor drag delta to nearest increment from Preferences; show on status bar |
| Precise input | T/R/S → type → Enter | Input buffer intercepts digits/./−/backspace while gizmo active; axis lock with X/Y/Z |
| Pivot point | Viewport dropdown | Individual origins / Median / 3D cursor / Active element; adjusts gizmo center + transform math |

## Edit Mode
| Feature | Shortcut | One-line spec |
|---|---|---|
| Object ↔ Edit mode | Tab | Enter: copy mesh → `EditMesh`; Exit: `Mesh::UpdateData` + clear |
| Element select | 1/2/3 | Vertex/Edge/Face mode; B=box, C=circle, Ctrl+RMB=lasso; Shift=additive |
| Extrude | E | Dupe selected + connect with new faces; enters grab; Escape reverts |
| Inset faces | I | Shrink each selected face inward by thickness (mouse drag); I again = toggle individual |
| Loop cut | Ctrl+R | Scroll=count; click=confirm; slide=position; requires quad face loop traversal |
| Bevel | Ctrl+B / Ctrl+Shift+B | Edges/vertices; width+segments+profile params; scroll=segments |
| Merge/weld | M → submenu | At center/cursor/first/last; By Distance auto-merges within threshold |
| Delete | X or Del | Vertices/edges/faces/dissolve/limited dissolve — different topology results |
| Fill | F | 2 verts→edge; closed loop→n-gon; Shift+F=grid fill between two parallel loops |
| Knife | K | Click to place cut points across faces; Enter=confirm; C=angle constraint (45°/90°) |
| Proportional editing | O (toggle) | Nearby unselected verts move with falloff; scroll=radius; Shift+O=falloff shape |
| Flip normals | Alt+N | Reverse winding order of selected faces; recompute normals |
| Recalculate normals | Shift+N | Flood-fill consistent outward winding from seed face |
| Subdivide | RMB menu | Split each selected face into 4; cuts/smoothness/fractal params; destructive |

## Scene & Objects
| Feature | Shortcut | One-line spec |
|---|---|---|
| Save / Load | Ctrl+S / Ctrl+O | JSON: entities, components, camera, asset paths (relative to root) |
| Export | File > Export | .obj or .gltf via `aiExportScene`; flatten modifiers; choose selected/all |
| Duplicate | Shift+D | New entity, same `AssetHandle`; enters grab; undoable as CreateEntity command |
| Parent/child | Ctrl+P / Outliner drag | `ParentComponent`; local-space `TransformComponent`; DFS world transforms in `RenderingSystem` |
| Visibility | H / Alt+H | `VisibilityComponent`; excluded from render + picking; grayed in Outliner |
| 3D cursor | Shift+RMB | `Vec3` in `EditorLayer`; used as pivot, snap target, new-object placement |
| Origin to geometry | RMB → Set Origin | Shift all verts by −delta; move origin by +delta; world pos unchanged |
| Apply transforms | Ctrl+A | Bake T/R/S into vertex data; reset component to identity |

## Camera & Viewport
| Feature | Shortcut | One-line spec |
|---|---|---|
| Orthographic views | Numpad 1/3/7/5 | Snap camera to axis + ortho projection; scale rulers on edges |
| Multi-viewport | Corner drag | Multiple `Framebuffer` instances; input routed to focused pane |
| Camera object | Add > Camera; Numpad 0 | `CameraComponent` entity; frustum gizmo; Numpad 0 pilots through it |
| Fly/walk mode | Shift+` | FPS yaw/pitch; W/A/S/D; `GLFW_CURSOR_DISABLED`; Escape to exit |
| Frame selected | Numpad `.` or F | Fit camera to selection bounding box; animated transition |

## Lighting
| Feature | Menu | One-line spec |
|---|---|---|
| Point light | Add > Light > Point | `PointLightComponent`; omnidirectional; inverse-square falloff; `LightUBO` |
| Directional light | Add > Light > Directional | Parallel rays; direction from `TransformComponent.EulerDegrees` |
| Spot light | Add > Light > Spot | Cone; inner/outer angle; `smoothstep` penumbra in shader |
| Ambient light | Scene panel | Global `vec3 ambient` uniform; typically 0.05–0.2 intensity |
| Shadow maps | CastShadows toggle | Depth-only FBO from light POV; PCF sampling in main pass |
| Light gizmo | auto on select | Point=sphere wire; Dir=arrow bundle; Spot=cone wire |

## Materials & Shading
| Feature | One-line spec |
|---|---|
| Material editor panel | Inspector tab; create named `Material` assets; per-slot editing; sphere preview |
| PBR params | Albedo, Metallic, Roughness, AO; Cook-Torrance BRDF in `pbr.shader` |
| Normal map | Per-vertex tangent via Assimp; TBN matrix in VS; decode RGB→[-1,1] in FS |
| Multiple UV channels | `texcoord0` + `texcoord1` in `Vertex`; `mTextureCoords[1]` from Assimp |
| Material slots | `face.mMaterialIndex` → `vector<shared_ptr<Material>>`; one draw call per slot |
| Emissive | `emissiveColor * intensity` added after light loop; feeds bloom bright-pass |
| Skybox / HDR | Cubemap or equirectangular→cubemap conversion; IBL irradiance + specular mip chain |

## UV Editing
| Feature | Shortcut | One-line spec |
|---|---|---|
| UV unwrap | U in Edit mode | LSCM or ABF algorithm; seams control UV islands |
| UV editor panel | — | 2D panel; select UV elements; G/R/S transform UVs; texture background |
| Seam marking | Ctrl+E | `bool isSeam` per edge in `EditMesh`; renders red in viewport; UV island boundary |
| Pack islands | U > Pack Islands | Bin-pack all UV islands into 0–1 square; margin configurable |

## Modifiers
| Modifier | One-line spec |
|---|---|
| Modifier stack | `vector<unique_ptr<IModifier>>`; non-destructive; evaluated when `ModifiersDirty` |
| Subdivision surface | Catmull-Clark; level 0–6; per-edge crease weight; or Loop subdivision (simpler) |
| Mirror | Mirror across X/Y/Z; Merge threshold weld; Clipping prevents crossing mirror plane |
| Array | Count + fixed/relative/object offset; appends incrementally translated copies |
| Solidify | Extrude thin surface to shell; thickness + offset + rim fill |
| Smooth | Iterative Laplacian relaxation; `v_new = v + factor*(avg(neighbors)−v)` |

## Selection & Productivity
| Feature | Shortcut | One-line spec |
|---|---|---|
| Box select (edit) | B | Screen-space rect → test projected element positions; Shift=additive |
| Select linked | L / Ctrl+L | BFS from hovered/selected through shared edges |
| Select all/deselect | A (toggle) | All elements in current mode |
| Invert selection | Ctrl+I | Flip selected ↔ unselected |
| Hide/unhide | H / Alt+H | `bool hidden` per element; excluded from render+pick; restored on Alt+H |
| Search / command palette | F3 | Fuzzy-search all registered actions; ImGui popup + InputText + ListBox |

## Rendering & Display
| Feature | One-line spec |
|---|---|
| Viewport shading modes | Z pie: Wireframe / Solid / Material Preview / Rendered → writes `ViewportShadingMode` enum |
| SSAO | G-buffer → hemisphere samples → bilateral blur → multiply ambient |
| Bloom | Bright-pass → Gaussian blur chain → additive composite before tone map |
| FXAA / MSAA | FXAA = full-screen shader pass; MSAA = `GL_TEXTURE_2D_MULTISAMPLE` FBO |
| Overlay toggles | Face normals, vertex normals, edge creases, vertex weights, face centers — `GL_LINES/GL_POINTS` |
| Measurement overlay | Edge length / face area / dihedral angle as ImGui draw list text projected to screen |

## Tooling & UX
| Feature | Shortcut | One-line spec |
|---|---|---|
| Preferences | Ctrl+, | Plain struct → `~/.config/opengl-modeler/prefs.json`; theme/font/autosave/snap/shadow res |
| Keymap editor | Preferences > Keymap | `flat_map<ActionID,KeyBinding>`; click-to-rebind; conflict highlighting |
| Autosave | Background timer | Write to `~/.local/share/opengl-modeler/autosave/`; recovery dialog on launch |
| Recent files | File > Open Recent | Last N paths in prefs JSON; grayed if path missing |
| Drag & drop import | Drop on viewport | `glfwSetDropCallback`; dispatch by extension to `AssetManager::Load` |
