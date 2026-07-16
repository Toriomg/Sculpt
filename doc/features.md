# 3D Modeler — Feature Roadmap

Reference for what the project needs to become a functional 3D modeler.
Grouped by domain; roughly priority-ordered within each group.

---

## Already Implemented

| Feature | Notes |
|---|---|
| 8 mesh primitives | Cube, Sphere, Pyramid, Torus, Cone, Arrow, Dodecahedron, Icosahedron |
| GPU picking | Entity ID encoded per-pixel; click-to-select |
| Multi-select | Shift+click; selection highlight shader |
| Translation gizmo | T key, RGB axis handles |
| Rotation gizmo | R key |
| Undo / Redo | Command stack (100 deep), Ctrl+Z / Ctrl+Y |
| Orbit camera | Middle-mouse drag, scroll-wheel dolly |
| Infinite grid | Ray-cast Y=0, multi-scale |
| Mesh import | .obj via Assimp |
| Texture import | .png / .jpg via stb_image |
| UI panels | Outliner, Inspector, Viewport, Menu bar |
| Wireframe toggle | Per-mesh |
| ECS architecture | EnTT registry; Name, Transform, Mesh, Camera, Selection components |

---

## Transform & Gizmos

### Scale gizmo
**Shortcut:** S key (with optional X/Y/Z axis lock)

The third pillar of the standard T/R/S transform triad, currently missing. Displays three axis-aligned handles extending from the object's origin plus a center cube for uniform scale. Dragging a single handle scales only along that axis; dragging the center cube scales all three axes proportionally. The gizmo reads and writes the scale component of the `TransformComponent` matrix. Must integrate with the existing undo/redo command stack — the scale is recorded on mouse-release, same pattern as the translation and rotation gizmos.

---

### Gizmo space toggle
**Shortcut:** G key (cycles Global → Local → Normal)

Controls the orientation of the axis handles for all three gizmos. In **Global** space the handles always align with the world X/Y/Z axes regardless of the object's rotation — useful when you know the absolute direction to move. In **Local** space the handles follow the object's own rotation — useful after rotating an object and wanting to push it "forward" in its own frame. **Normal** space (relevant in Edit mode) aligns to the average normal of the selected elements. The current gizmo renders hardcoded world-space axes; this feature requires passing the object's rotation matrix (or the selection's normal) to the gizmo shader and to the drag-projection math.

---

### Snap
**Shortcut:** Hold Ctrl while dragging any gizmo

Constrains the output value of a drag to discrete increments, preventing sub-increment movements. Three variants:
- **Translate snap** — rounds the position to the nearest grid unit (e.g. 0.25, 0.5, 1.0 m). The snap increment should be configurable in Preferences and displayed on the status bar while snapping.
- **Rotate snap** — locks rotation to a degree multiple (commonly 5°, 15°, or 45°). Useful for exact 90° turns.
- **Scale snap** — quantizes scale to steps (e.g. 0.1, 0.25, 0.5). Prevents accidentally landing on a scale like 1.317×.

Implementation: the drag delta is computed as normal, then floored to the nearest multiple of the snap increment before being applied to the transform. The snap increment state lives in the editor settings struct that Preferences already partially owns.

---

### Precise input
**Shortcut:** After pressing T / R / S, type a number then Enter

Lets the user bypass the mouse entirely and type an exact transform value. The workflow: press T, the translation gizmo activates; press X (optional, to lock axis), type `2.5`, press Enter — the object moves exactly 2.5 units on X. This is critical for technical modeling where eyeballing is insufficient. Requires a small input buffer that intercepts keyboard digits, `.`, `-`, and backspace while a gizmo is active. The axis lock (X/Y/Z keys) works the same way it does for mouse drags; the typed value replaces the mouse delta.

---

### Pivot point selector
**UI:** Dropdown in the viewport header bar

Controls the point around which rotations and scales are applied when multiple objects are selected. Options:
- **Individual origins** — each selected object rotates/scales around its own origin independently. Default for most multi-object operations.
- **Median point** — transforms happen around the geometric center of all selected objects' origins. Useful for rotating a group symmetrically.
- **3D cursor** — uses the 3D cursor position as the pivot. Allows rotating around an arbitrary point in space.
- **Active element** — uses the last-selected object's origin as the pivot for all selected objects.

Requires storing the chosen mode in the editor state and adjusting the gizmo's center position and the math in the transform command before applying it.

---

## Mesh Editing (Edit Mode)

Currently the editor only operates on whole objects (Object Mode). Mesh editing is the core missing domain — it requires a separate editing state machine with its own data structures, selection tracking, and rendering path for vertices, edges, and faces.

### Edit mode / Object mode toggle
**Shortcut:** Tab key

Switches the editor between two distinct states:

- **Object Mode** — the current behavior. Entities are selected and transformed as whole units. The gizmos act on the `TransformComponent` matrix.
- **Edit Mode** — the active object's mesh is decomposed into selectable vertices, edges, and faces. Gizmos act on the raw vertex positions. The mesh data must be copied into a CPU-side editable representation (a half-edge or indexed triangle structure) on entering Edit mode, and written back to the GPU VBO on exit or after each operation.

The visual difference: in Edit Mode the viewport renders the mesh with vertex dots, edge lines, and face highlights overlaid on top of the solid shading. The Outliner and Inspector update to show element-level information instead of object-level components.

---

### Element selection (Vertex / Edge / Face)
**Shortcuts:** 1 = Vertex, 2 = Edge, 3 = Face mode; B = Box select, C = Circle select

The GPU picking system encodes entity IDs; in Edit Mode it must be extended to encode per-element IDs (primitive index from `PickingTexture` already stores `PrimitiveID` — this is the hook). From the primitive ID the CPU resolves which vertex, edge, or face was hit.

Selection modes:
- **Vertex mode** — clicking selects a vertex. The selected set is a `std::vector<uint32_t>` of vertex indices.
- **Edge mode** — selects an edge (pair of vertex indices). The picker returns the closest edge to the clicked primitive.
- **Face mode** — selects a face (a polygon / triangle group). The primitive ID maps directly to a face index.

**Box select (B):** drag a rectangle in the viewport; all elements whose screen-projected position falls inside are added to or set as the selection.
**Circle select (C):** hold and move the mouse; elements within a radius circle are continuously selected. Scroll wheel changes the radius.
**Lasso:** Ctrl+right-click drag draws a freeform shape; elements inside are selected.
All three must share the same "additive vs. replace" toggle (Shift for additive).

---

### Extrude
**Shortcut:** E key

Duplicates selected geometry and connects it to the original with new faces, then immediately enters a grab (translate) operation so the user drags the extruded region into place.

- **Face extrude** — the most common case. Selected faces are copied; their boundary edges become the connecting "walls". The extruded copy moves along the averaged face normal by default, or along an axis if X/Y/Z is pressed.
- **Edge extrude** — selected edges duplicate into new faces; the duplicate edges are grabbed.
- **Vertex extrude** — selected vertices duplicate into new edges.

After the drag the result is committed to the mesh and pushed to the undo stack as a single command. If the user presses Escape, the duplicated geometry is deleted and the original is restored.

---

### Inset faces
**Shortcut:** I key

Shrinks each selected face inward from its boundary, creating a new inner face surrounded by a border of new faces. Useful for adding detail (a panel inset, a window recess, a bolt hole setup) without manually placing vertices.

Parameters (shown in a small HUD while the operation is active):
- **Thickness** — how far inward the boundary moves; controlled by mouse drag after pressing I.
- **Individual** — toggle (I again) between insetting all selected faces as a group vs. each face independently around its own center.

Implementation: for each selected face, compute an inset polygon by moving each vertex inward along the face's averaged edge directions, then stitch the inset polygon and the original boundary with quads.

---

### Loop cut
**Shortcut:** Ctrl+R, then scroll wheel to set count, then click to confirm, then slide to position

Inserts one or more edge loops — continuous rings of edges that run around a mesh following a "flow" of parallel quads. This is the primary topology management tool.

Workflow:
1. Hover over an edge: the tool highlights the face loop it would cut (the sequence of quad faces that edge belongs to) in orange.
2. Scroll wheel increases the cut count (1 loop, 2 loops, …).
3. Click confirms the position; the cut is placed at the center by default.
4. Mouse movement slides the cut toward either endpoint of the edge strip. Right-click or Enter locks it.

Requires the mesh to support finding edge loops (chains of edges where each step goes across the opposite side of a quad face — standard half-edge traversal).

---

### Bevel
**Shortcut:** Ctrl+B (edges), Ctrl+Shift+B (vertices)

Rounds sharp edges or vertices by replacing them with a strip of new faces, distributing the original sharp angle across multiple smaller faces. Essential for making hard-surface models look physically plausible — infinitely sharp edges don't exist on real objects.

Parameters (mouse drag to adjust, scroll to change segments):
- **Width** — how far the bevel extends from the original edge. Controlled by mouse distance from click point.
- **Segments** — how many edge loops are inserted within the bevel strip. 1 = chamfer, 2+ = rounded profile.
- **Profile** — the curvature of the bevel cross-section (0 = concave, 0.5 = flat chamfer, 1 = convex).

Vertex bevel (Ctrl+Shift+B) cuts across the corner of a vertex, replacing it with a polygon.

---

### Merge / Weld vertices
**Shortcut:** M key → submenu

Collapses multiple selected vertices into a single vertex, connecting all their edges to the survivor. Used to close holes, fix duplicate vertices at seams, or simplify topology.

Merge targets:
- **At Center** — the new vertex is placed at the arithmetic mean of all selected vertices' positions.
- **At Cursor** — the new vertex is placed at the 3D cursor.
- **At First / At Last** — the new vertex is at whichever vertex was selected first or last.
- **By Distance (weld)** — automatically merges any two vertices within a threshold distance. Equivalent to "Remove Doubles" — critical after importing or joining meshes that may have coincident vertices at seams.

---

### Delete elements
**Shortcut:** X or Delete key → submenu

Removes selected geometry. Deletion has different topological meanings depending on what's selected:
- **Vertices** — removes the vertex and all edges and faces that used it. Can leave holes.
- **Edges** — removes the edge and dissolves any faces that used it (faces become n-gons or disappear).
- **Faces** — removes the face polygons but leaves the boundary edges and vertices.
- **Dissolve vertices/edges** — removes the element and merges the surrounding geometry cleanly, without leaving holes. This is usually preferable to raw delete for topology cleanup.
- **Limited dissolve** — removes edges whose faces are coplanar within an angle threshold, simplifying flat regions into fewer polygons.

---

### Fill
**Shortcut:** F key

Creates new face geometry to close an open boundary:
- Select two vertices → creates an edge between them.
- Select a closed edge loop (boundary of a hole) → fills it with a single n-gon or a fan of triangles.
- Select multiple boundary loops → fills each with a face.
- **Grid fill** (Shift+F on two parallel edge loops of equal count) — fills the space between them with a clean grid of quads.

Implementation: check if selected edges form one or more closed loops; if so, triangulate the interior using an ear-clipping or Delaunay algorithm and append the new faces to the mesh.

---

### Knife tool
**Shortcut:** K key

Allows drawing arbitrary cut lines across faces, inserting new vertices and edges wherever the line crosses existing edges or faces. More flexible than loop cut because it is not restricted to following existing edge loops.

Workflow:
1. Press K; the cursor changes to a knife icon.
2. Click to place cut points. Each click places a vertex; a line preview follows the mouse.
3. Clicking on an existing edge snaps to it and inserts a vertex there.
4. Press Enter or Space to confirm all cuts. Press Escape to cancel.
5. **Angle constraint** (C key while active) forces the cut to exactly 45° or 90° from the previous point.

The result is that new edges split the intersected faces into smaller polygons. All new faces must be valid (no zero-area, no flipped normals).

---

### Proportional editing
**Shortcut:** O key toggle; scroll wheel changes falloff radius while active

When a transform (grab, rotate, scale) is applied to selected elements, proportional editing also moves the *unselected* nearby elements, with influence fading according to a falloff curve. This produces smooth, organic deformations instead of hard-edged movement of only the selected region.

Falloff shapes (cycled with Shift+O):
- **Smooth** — sine-based, the most natural feeling.
- **Sphere** — linear from center to radius edge.
- **Root** — square-root curve; stronger near the center.
- **Sharp** / **Linear** / **Constant** — progressively harder falloffs.

The radius is shown as a circle overlay in the viewport. Implementation: after computing the delta for selected elements, iterate over all mesh vertices, compute their distance to the nearest selected vertex, and if within radius apply the delta multiplied by `falloff(distance / radius)`.

---

### Flip normals
**Shortcut:** Alt+N → Flip

Reverses the winding order (and thus the face normal) of every selected face. Used when a face is inside-out — its normal points inward, making it invisible from the outside or lit incorrectly. Common after importing geometry from tools with different winding conventions or after certain extrude operations. Implementation: for each selected face, swap the order of its vertex index list (e.g. reverse the index array), then recompute and upload the normal buffer.

---

### Recalculate normals
**Shortcut:** Shift+N

Automatically determines the "outward" direction for all selected faces by analyzing mesh connectivity and propagating consistent winding from a seed face. Works by:
1. Building a face adjacency graph.
2. Starting from any face, flood-filling through shared edges.
3. At each edge, checking whether the neighboring face's winding agrees or disagrees with the current face; if it disagrees, flipping it.
4. Repeating until all connected faces are consistent.

For closed meshes (a sphere, a cube) this produces outward-pointing normals. For open meshes or meshes with non-manifold edges the behavior is best-effort. Useful as a bulk repair after complex edit operations that may have left inconsistent winding.

---

### Subdivide
**Shortcut:** Right-click context menu → Subdivide (in Edit mode, on selected faces)

Splits each selected face into 4 smaller faces by inserting a vertex at the center and at the midpoint of each edge. Increases mesh resolution uniformly in the selected region, giving more vertices to work with for detail sculpting without switching to a non-destructive modifier.

Parameters:
- **Number of cuts** — how many times to subdivide (1 cut = 4×, 2 cuts = 16×, etc.).
- **Smoothness** — 0 = flat (new vertices stay on original face planes), 1 = Catmull-Clark-like smoothing moves new vertices toward a smooth surface.
- **Fractal** — adds random displacement to new vertices; useful for organic or terrain-like geometry.

Unlike the Subdivision Surface modifier (non-destructive), this operation is destructive and permanently adds vertices to the mesh data.

---

## Scene & Objects

### Scene save / load
**Menu:** File > Save (Ctrl+S), File > Open (Ctrl+O)

Persists the full editor state to disk and restores it exactly. The scene file must encode:
- Every entity and its component set: name, transform matrix, mesh reference, material reference.
- Asset paths (relative to the project root) for meshes and textures — not the GPU handles, which are runtime-only.
- Camera position and settings.
- Editor-level state: selected entities, active layer visibility.

**Format choice:** JSON is human-readable and debuggable (recommended for development); a binary format (e.g. MessagePack) is faster for large scenes. Start with JSON. The serialization layer should live outside `EditorLayer` to remain testable. On load, assets are re-loaded through `AssetManager::Load()` so the handle system is repopulated correctly.

---

### Export
**Menu:** File > Export > Wavefront (.obj) / glTF 2.0 (.gltf)

Writes the current mesh (or selected meshes) to a format usable by other tools. Unlike Save, Export is a one-way conversion — it flattens the current state of all modifiers and produces final geometry.

- **.obj export** — straightforward: write vertex positions, normals, UVs, and face indices in the Wavefront format. Assimp can handle this via `aiExportScene`; no custom writer needed.
- **.gltf export** — the modern standard; supports PBR materials, multiple meshes, and a scene hierarchy in a single file. Also doable via Assimp or a minimal hand-written JSON+binary glTF writer.

The export dialog should let the user choose which objects to include (selected only vs. all), the coordinate system convention (Y-up vs. Z-up), and the output path.

---

### Duplicate object
**Shortcut:** Shift+D

Creates an independent copy of the selected entity (or entities) with a fresh transform, then immediately enters grab mode so the user can place the copy. The duplicate shares the same mesh and material assets (same `AssetHandle`) — it is not a deep copy of the geometry, just a new entity pointing to the same resources. If the user later edits the mesh in Edit Mode, a "Make Single User" action should deep-copy the mesh data first (linked duplicates vs. real duplicates, like Blender's Alt+D vs. Shift+D). The new entity is added to the undo stack as a "Create Entity" command so it can be undone cleanly.

---

### Parent / child hierarchy
**UI:** Drag one entity onto another in the Outliner; Ctrl+P shortcut in viewport

Establishes a transform parent-child relationship: the child's world transform is computed as `parent_world * child_local`. Moving or rotating the parent carries the child along; editing the child's local transform leaves the parent unchanged.

Implementation requirements:
- Add a `ParentComponent` (stores the parent `entt::entity`) and update `TransformComponent` to store a local-space matrix.
- The `RenderingSystem` must do a DFS over the hierarchy each frame to compute world transforms before submitting draw calls.
- The Outliner must render the tree indented to show depth; drag-and-drop reorders the `ParentComponent`.
- "Unparent" (Alt+P) removes `ParentComponent` and bakes the current world transform back into the entity's local transform so it doesn't jump.

---

### Object visibility toggle
**UI:** Eye icon per row in the Outliner

Hides an object from the viewport render without deleting it. Hidden objects are excluded from the `RenderingSystem` query and from the picking pass. Their data (mesh, material) remains loaded. The visibility state is a boolean in a `VisibilityComponent` (or a flag in `MeshComponent`). Hidden objects should still appear in the Outliner (grayed out) so they can be found and restored. Keyboard shortcut: H to hide selected, Alt+H to unhide all.

---

### 3D cursor
**Shortcut:** Shift+right-click places the cursor; Shift+C resets it to origin

A special marker in the 3D viewport that serves as a reference point for several operations. It is not an entity — it is editor state (a `Vec3` position stored in `EditorLayer`). Its uses:
- **Pivot point** — when pivot mode is set to "3D cursor", rotations and scales happen around the cursor instead of object origins.
- **Snap target** — objects or vertices can be snapped to the cursor position.
- **New object placement** — newly added primitives appear at the cursor location instead of the world origin.

Visually it is rendered as a crosshair/circle icon using a simple line-draw call, not as a mesh entity.

---

### Origin to geometry
**Menu:** Right-click → Set Origin → Origin to Geometry

Recenters an object's local origin to its mesh's center of mass (average of all vertex positions, or optionally the bounding box center). The mesh vertices are offset by the inverse of the shift so the geometry doesn't move in world space — only the origin moves.

This matters because all gizmos, rotation pivots, and parent-child transforms operate relative to the origin. An object with a badly placed origin (e.g. one imported with the origin at a distant point) is painful to work with. Related operations: "Origin to Cursor" (moves origin to 3D cursor), "Geometry to Origin" (moves the mesh vertices instead, keeping origin fixed).

---

### Apply transforms
**Shortcut:** Ctrl+A → submenu (Location / Rotation / Scale / All)

Bakes the current transform values into the mesh vertex data and resets the corresponding component to its identity value (position → 0,0,0; rotation → identity; scale → 1,1,1). Useful when a model's scale is, say, 2× but child objects, physics, or modifiers expect scale 1 — applying scale multiplies all vertex positions by 2 and resets the scale component to 1, making the visual result identical but the data clean.

Requires CPU-side access to the mesh vertex buffer, multiplication of all positions by the transform, re-upload to the VBO, and reset of the `TransformComponent` matrix.

---

## Camera & Viewport

### Orthographic views
**Shortcuts:** Numpad 1 (Front), Numpad 3 (Right), Numpad 7 (Top), Numpad 5 (toggle Ortho/Persp)

Snaps the viewport camera to a standard axis-aligned view and switches to orthographic projection. In orthographic mode, parallel lines remain parallel (no perspective foreshortening), which is essential for precision modeling — checking that geometry is truly flat, aligning objects by eye, or modeling by reference image.

Implementation: store a separate "orthographic camera" state in `EditorLayer` with a `Camera::SetOrthographic(left, right, bottom, top, near, far)` path. Numpad shortcuts write the camera's position and orientation to face the corresponding axis. Ortho view should display scale rulers along the edges of the viewport. The current perspective camera remains intact; toggling Numpad 5 swaps between the two projection matrices without changing the view direction.

---

### Multi-viewport
**UI:** Drag the corner of the viewport to split; menu View > Viewport Layout

Splits the main viewport area into 2 or 4 independent panes. Each pane has its own camera, shading mode, and overlay settings. The classic layout is four panes: top-left = Top ortho, top-right = Front ortho, bottom-left = Right ortho, bottom-right = Perspective — the "quad view" standard in almost every 3D application.

Implementation: `EditorLayer` currently draws into one framebuffer. Multi-viewport requires multiple `Framebuffer` instances (one per pane), a layout manager that determines each pane's screen rect, and forwarding input events to the focused pane only. The pane that last received a click is "active"; gizmo interaction goes to the active pane.

---

### Camera object
**Menu:** Add > Camera; **Shortcut:** Numpad 0 looks through the selected camera

Allows placing one or more Camera entities in the scene (beyond the editor fly-camera that always exists). A Camera entity has a `CameraComponent` with its own projection settings (FOV, near/far), and its position is controlled by a `TransformComponent` like any other object. Pressing Numpad 0 temporarily pilots the viewport through that camera's lens — what you see is what a renderer would see from that position. Useful for composing render shots or for games where the camera is a scene object.

Already partially supported: `CameraComponent` and `RenderingSystem` look for a primary camera. The missing piece is: (1) adding Camera as a creatable primitive in the Add menu, (2) a visual frustum gizmo drawn in the viewport when the camera entity is selected, (3) Numpad 0 piloting.

---

### Fly / walk navigation
**Shortcut:** Shift+` (backtick) to enter; Escape or RMB to exit

A first-person navigation mode where the viewport camera is controlled like a game character. While active: W/A/S/D moves forward/left/backward/right relative to the view direction; Q/E moves down/up; mouse look rotates the view. Scroll wheel or Shift changes movement speed. This mode is useful for navigating large scenes, for previewing a game-camera path, or simply when orbit navigation feels awkward inside an enclosed space.

Implementation: a boolean `m_FlyMode` flag in `EditorLayer`. When active, the camera controller switches from orbit math to a pure FPS yaw/pitch accumulator. Mouse cursor is locked (hidden and kept centered via `glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED)`).

---

### Frame selected
**Shortcut:** Numpad period (`.`) or F (with viewport focused)

Recenters and resizes the viewport camera so that all selected objects (or the whole scene if nothing is selected) fill the viewport comfortably. Computes the bounding box of the selection, positions the camera to face the box center at a distance proportional to the box diagonal, and preserves the current view direction (just adjusts distance and pivot). Animates the transition over a few frames for spatial context. Essential workflow shortcut — without it, users lose objects off-screen and have no fast way to find them.

---

## Lighting

### Point light
**Menu:** Add > Light > Point

An omnidirectional light that emits equally in all directions from a single position. Simulates a light bulb, candle, or any small local light source.

Parameters (editable in the Inspector):
- **Color** — RGB tint of the emitted light.
- **Intensity** — scalar multiplier; affects how bright the light is at the falloff boundary.
- **Radius** — the distance at which the light's contribution reaches zero. Beyond this radius no computation is done (cheap early-out in the shader).
- **Falloff** — physically correct lights use inverse-square falloff (`1 / d²`); a linear or constant falloff is cheaper but less realistic.

Implementation: add a `PointLightComponent` struct. The `RenderingSystem` (or a dedicated `LightingSystem`) collects all light components into a UBO (Uniform Buffer Object) uploaded once per frame, replacing the current hardcoded ambient+diffuse in the mesh shader. The mesh shader iterates the light array and accumulates contributions.

---

### Directional light
**Menu:** Add > Light > Directional

A light with no position — only a direction. All light rays are parallel (like sunlight at distance). Illuminates everything in the scene from the same angle regardless of position.

Parameters:
- **Direction** — a unit vector (editable via a rotation gizmo in the viewport, or Euler angles in the Inspector).
- **Color** and **Intensity**.
- **Cast shadows** — toggle that enables shadow map generation for this light (see Shadow maps feature).

A directional light entity is represented in the viewport by a visual arrow gizmo (or a sun disc icon) so it can be selected and repositioned. Its `TransformComponent` rotation determines the light direction.

---

### Spot light
**Menu:** Add > Light > Spot

A cone-shaped light that emits from a point in one direction, like a flashlight or stage spotlight. More complex than a point light because it has both position and direction.

Parameters:
- **Inner angle** — within this cone the light is at full intensity.
- **Outer angle** — between inner and outer the intensity fades (penumbra/soft edge).
- **Color**, **Intensity**, **Falloff radius**.
- **Cast shadows** — spot lights are the most natural shadow casters for interior scenes.

In the shader, the spotlight attenuation is computed as: check if the fragment-to-light direction falls within the cone, then compute the smooth fade between inner and outer angle using `smoothstep`.

---

### Ambient light
**UI:** Scene panel → Ambient section

A global, directionless light that adds a flat base illumination to all surfaces equally, regardless of their orientation or distance from any light. Prevents fully-unlit faces from being pure black (which looks harsh and loses surface detail in shadow).

Parameters:
- **Color** — the tint of the ambient contribution. Often a desaturated sky blue to mimic sky bounce.
- **Intensity** — scalar multiplier, typically kept small (0.05 – 0.2).

The simplest implementation is a `vec3 ambient` uniform sent to the mesh shader each frame. A physically-based upgrade is image-based ambient via a skybox (see Skybox feature).

---

### Shadow maps
**Prerequisite:** at least one directional or spot light with "Cast shadows" enabled

Renders the scene from the light's point of view into a depth texture, then in the main render pass samples that depth texture to determine whether each fragment is occluded (in shadow) or lit.

Steps:
1. **Shadow pass** — bind a depth-only FBO; render all opaque meshes with a simple depth-write shader from the light's view+projection matrix (orthographic for directional, perspective for spot). Store the result as a `sampler2DShadow`.
2. **Main pass** — in the mesh fragment shader, transform each fragment into the light's clip space, sample the shadow map with `textureProj` + PCF (Percentage Closer Filtering) to get a soft shadow factor, multiply the light contribution by that factor.

PCF averages multiple shadow map samples in a small kernel (e.g. 3×3 or 5×5) to soften the shadow edge, hiding aliasing from the depth texture resolution. The shadow map resolution (e.g. 2048×2048) and PCF kernel size should be configurable in Preferences.

---

### Light gizmo
**Visual:** In-viewport icon/handle drawn over light entities when selected

Provides visual feedback for where lights are and what direction they point, since a light entity has no mesh geometry. Without gizmos, lights are invisible in the viewport.

- **Point light** — a sphere wireframe or a star-burst icon centered at the light's position.
- **Directional light** — a parallel-ray arrow bundle or a sun disc with rays.
- **Spot light** — a cone wireframe matching the inner and outer angle; apex at the light's origin.

Gizmos are drawn with `RenderCommand` using line geometry (no texture needed) after the main scene pass but before the selection highlight pass. They should only be visible when the light entity is selected, or optionally as a subtle always-visible icon in a lighter color.

---

## Materials & Shading

### Material editor panel
**UI:** Dedicated "Material" tab in the Inspector (replaces the current single texture path field)

A panel that exposes all material parameters for the selected object, supports creating and naming material assets, and allows assigning multiple materials to a single mesh via material slots. The current implementation hardcodes one texture per mesh; this replaces that with a proper material system.

The panel shows:
- A dropdown to select which material slot is being edited.
- A "New Material" button that creates a named `Material` asset in the `AssetManager`.
- Per-parameter controls (color pickers, float sliders, texture path fields with a browse button).
- A small sphere preview rendering the material under a default lighting setup.

---

### PBR parameters
**Shader:** Extend `modelmesh.shader` to a full PBR pipeline

Physically Based Rendering uses a small set of physically motivated parameters to describe how a surface interacts with light, producing realistic results across all lighting conditions.

Parameters:
- **Albedo** — the base color of the surface (replaces the current diffuse color). Can be a flat color or a texture.
- **Metallic** — 0 = dielectric (plastic, wood, skin), 1 = conductor (gold, iron). Controls how albedo and specular are mixed.
- **Roughness** — 0 = perfectly smooth (mirror), 1 = fully diffuse. Controls the width of specular highlights via the GGX NDF.
- **Ambient Occlusion (AO)** — a texture (or baked value) that darkens crevices that receive less ambient light.

The shader uses the Cook-Torrance BRDF: `(D * F * G) / (4 * NdotV * NdotL)`. This is well-documented and the GLSL implementation is a standard ~40-line addition to the fragment shader.

---

### Normal map
**Inspector:** "Normal Map" texture slot in the Material editor

A texture whose RGB channels encode a surface normal in tangent space (R=X, G=Y, B=Z, with 0.5 being the "flat" value). Sampling this texture in the fragment shader replaces the geometry's interpolated normal with a perturbed one, creating the illusion of fine surface detail (bumps, rivets, scratches) without adding geometry.

Implementation requires:
1. The mesh must store a tangent vector per vertex (Assimp can compute these on load via `aiProcess_CalcTangentSpace`).
2. The vertex shader outputs the tangent, bitangent, and normal as a TBN matrix.
3. The fragment shader samples the normal map, decodes it to a `[-1, 1]` range vector, multiplies by TBN, and uses the result as the shading normal.

---

### Multiple UV channels
**Mesh data:** Store a second `texcoord1` attribute alongside `texcoord0`

A mesh can carry more than one set of UV coordinates. Common uses:
- **UV0** — detail texture UVs (may tile across the surface).
- **UV1** — lightmap UVs (unique per texel, no overlap, 0–1 range only).

Assimp supports up to `AI_MAX_NUMBER_OF_TEXTURECOORDS` UV channels; loading UV1 requires reading `mesh->mTextureCoords[1]`. The `Vertex` struct needs a second `Vec2 TexCoord1`. The shader accesses it via a second `in vec2 vTexCoord1` varying.

---

### Material slots
**Mesh data:** A face's material index maps to a slot in the object's material list

Allows assigning different materials to different regions of the same mesh. In the standard .obj format, mesh faces carry a `usemtl` material reference; Assimp exposes this as `aiFace::mMaterialIndex`. Each slot has its own draw call with its own shader state.

Implementation: `MeshComponent` stores a `std::vector<shared_ptr<Material>>` (material list) and the `Mesh` stores a `uint32_t` per face indicating which slot it belongs to. The `RenderingSystem` groups faces by material index and issues one `glDrawElements` per group (or uses a multi-draw indirect call with a material index pushed via UBO).

---

### Emissive
**Inspector:** Emissive color + intensity in the Material editor; optional emissive texture

A material can emit light of its own color independently of any scene lights. The surface renders at the emissive color even in a fully dark scene. Emissive materials are used for glowing surfaces: screens, LEDs, lava, neon signs.

In the shader: `vec3 emissive = emissiveColor * emissiveIntensity;` is added directly to the final fragment color after all lighting calculations, bypassing the light loop. If Bloom is implemented, the emissive value feeds into the bloom bright-pass threshold.

---

### Skybox / HDR environment
**Menu:** Scene > Environment > Load HDR or Load Cubemap

Replaces the solid background color with a panoramic environment. Two formats:
- **Cubemap** — six square textures (faces of a cube); rendered by drawing a cube with `glDepthFunc(GL_LEQUAL)` and a skybox shader that samples `samplerCube`.
- **HDR equirectangular** — a single `.hdr` or `.exr` panorama; converted at load time to a cubemap via a rendering pass that reprojects it.

The environment also drives **Image-Based Lighting (IBL)**: the HDR map is prefiltered into a diffuse irradiance map and a specular radiance map (at multiple mip levels), which replace the ambient light term in the PBR shader with physically correct environment lighting. This is a significant quality jump for metallic and smooth surfaces.

---

## UV Editing

### UV unwrap
**Shortcut:** In Edit Mode with faces selected → U key → Unwrap

Computes a 2D parameterization of the selected 3D surface — a mapping from every face's 3D position to a 2D UV coordinate — so that a 2D texture can be applied without distortion. The algorithm "unwraps" the mesh like unfolding a paper model.

Algorithm options:
- **Angle-Based Flattening (ABF)** — minimizes angle distortion; produces the most conformal (shape-preserving) result but is computationally expensive.
- **Least Squares Conformal Maps (LSCM)** — faster; minimizes both angle and area distortion simultaneously; the standard choice.

The UV seams (marked edges where the mesh is "cut open" for unfolding) control what the unwrap looks like. The result is UV islands stored in the mesh's UV channel, visible in the UV editor panel.

---

### UV editor panel
**UI:** A second panel (tab or split) showing the 2D UV space

A dedicated 2D editor where UV coordinates can be selected and transformed independently from the 3D viewport. The 0–1 UV square is shown with optional grid lines; UV islands are drawn as polygon outlines.

Selection in the UV editor mirrors the 3D viewport: selecting a face in the 3D view highlights the same face's UV island in the UV editor, and vice versa. Transform operations (G = move, R = rotate, S = scale) work on UV coordinates, updating the mesh's UV buffer. The background of the UV editor can optionally show the assigned texture, making it easy to align UVs to texture features.

---

### Seam marking
**Shortcut:** In Edit Mode with edges selected → U > Mark Seam / Clear Seam; or Ctrl+E → Mark Seam

Designates edges as "seams" — cut lines where the mesh will be opened during UV unwrap. Seams are analogous to the cut lines on a flat cardboard box: they determine how the 3D surface is unfolded into 2D. A closed mesh like a sphere must have at least one seam to unwrap at all.

Marked seams are rendered in the viewport as red (or highlighted) edge lines in Edit Mode. They are stored as an edge property (`bool isSeam`) in the editable mesh data structure. When `Unwrap` is run, the algorithm treats seams as boundaries, producing one UV island per connected region bounded by seams.

---

### Pack islands
**Shortcut:** UV Editor → U > Pack Islands

After unwrapping, UV islands may be scattered and differently scaled across UV space. Pack islands rearranges them to fit tightly inside the 0–1 square while keeping their relative proportions. This maximizes texture resolution usage — islands that are left small or spread out waste texel budget.

The algorithm is a bin-packing problem: rotate each island 0° or 90° (optionally arbitrary angles), then arrange them in the unit square to minimize wasted area. A simple shelf-packing heuristic is sufficient for a first implementation; an exact solver (e.g. xatlas library) produces significantly better packing but is complex. A configurable margin between islands prevents texel bleeding across island boundaries.

---

## Modifiers (non-destructive stack)

### Modifier stack
**UI:** "Modifiers" tab in the Inspector; each modifier is a collapsible card with an eye (toggle) and X (delete) button

The modifier stack is the architecture that makes all other modifiers non-destructive. Instead of editing mesh data directly, modifiers are applied in order at render time (and optionally at export time) to produce a final mesh, while the base mesh data remains unchanged.

Data flow: `base mesh data → Modifier 1 → Modifier 2 → ... → final mesh → GPU upload`.

Implementation: `MeshComponent` stores a `std::vector<unique_ptr<IModifier>>` alongside the base mesh. Each frame (or when the stack is "dirty") the system re-evaluates the chain: starts with the base vertex/index buffer, passes it through each modifier's `Apply(MeshData&)` method, and uploads the result. Modifiers can be reordered by drag-and-drop in the panel. "Apply" bakes the modifier into the base mesh and removes it from the stack.

---

### Subdivision surface
**Modifier:** Catmull-Clark subdivision; level 0–6 slider

The Catmull-Clark algorithm recursively refines a mesh, inserting new vertices at face centers and edge midpoints with weighting rules that cause the mesh to converge toward a smooth limit surface. Each subdivision level quadruples the face count, so level 1 is cheap, level 4 is expensive. The modifier is evaluated every frame (or lazily when dirty) at the configured level.

**Edge crease** — edges can be marked with a crease weight (0–1). A fully creased edge behaves as a sharp corner even at high subdivision levels, allowing hard edges on an otherwise smooth surface. Crease weights are stored as an edge property in the base mesh.

For a learning project, a simple Loop subdivision (triangle-based) is easier to implement than Catmull-Clark (quad-based) and is still educational.

---

### Mirror
**Modifier:** Mirror across X/Y/Z axes; "Merge" and "Clipping" options

Mirrors the base mesh across one or more axes through the object's origin, appending the mirrored geometry to produce a symmetric mesh. With "Merge" enabled, vertices that land within a threshold distance of the mirror plane are welded together, ensuring watertight seams. With "Clipping" enabled, vertices cannot be dragged across the mirror plane in Edit Mode — they are clamped at 0 on the mirrored axis, enforcing symmetry during editing.

This modifier is extremely commonly used for character modeling (model one half, mirror gives the other) and any symmetric hard-surface work. Implementation: duplicate all vertices with their X (or Y or Z) coordinate negated, reverse face winding on the mirrored copy (to fix normals), and conditionally merge boundary vertices.

---

### Array
**Modifier:** Count, offset mode (fixed offset, relative offset, or object offset)

Repeats the mesh a configurable number of times, placing each copy at an offset from the previous. Offset modes:
- **Fixed offset** — each copy is displaced by a constant `Vec3` in world space.
- **Relative offset** — each copy is displaced by a multiple of the mesh's own bounding box size along an axis (produces copies that tile perfectly with no gap).
- **Object offset** — uses the transform of another scene object to define the step between copies; allows curved arrays by animating the offset object.

Useful for fences, columns, stairs, chains — any repeated structure. Implementation: the modifier runs `count` times, appending the vertex and index buffers with incrementally translated copies of the base mesh.

---

### Solidify
**Modifier:** Thickness slider; offset (-1 to 1 centers the shell); Fill Rim toggle

Takes a thin surface mesh (e.g. a flat plane or a shell with only one side of faces) and extrudes it into a solid shell with a configurable thickness. For each face, a duplicate face is created at `normal * thickness` offset, and the boundary edges are closed with new "rim" faces connecting the two sides. The result is a manifold solid suitable for 3D printing or for representing real-world thin surfaces like sheet metal, cloth, or walls.

The "Offset" slider controls whether the shell grows toward the outside, inside, or equally both ways from the original surface.

---

### Smooth / Laplacian smooth
**Modifier:** Iterations slider (1–100); Factor slider (0–1); optional vertex weight mask

Iteratively relaxes vertex positions toward the average of their neighbors, removing high-frequency surface noise. Each iteration moves each vertex a `factor`-weighted step toward its one-ring neighbor average:

```
v_new = v + factor * (average(neighbors) - v)
```

Higher iteration counts or factors produce more smoothing. Laplacian smoothing (the formal name for this operation) can cause volume loss (the mesh shrinks) — a cotangent-weighted variant preserves volume better at the cost of more computation. A "vertex weight" mask painted on the mesh can limit smoothing to specific regions.

---

## Selection & Productivity

### Box select in Edit mode
**Shortcut:** B key, then drag

Draws a 2D rectangle in viewport space; all mesh elements (vertices, edges, or faces depending on the active element mode) whose screen-projected position falls inside the rectangle are added to the selection. The rectangle outline is drawn as an overlay on top of the 3D render while dragging. Releasing the mouse confirms the selection; Shift+B adds to existing selection, regular B replaces it. Already exists conceptually for object mode (the existing GPU picking handles single clicks); box select requires a screen-space rectangle test against all projected element positions, not a per-pixel GPU pass.

---

### Select linked
**Shortcut:** L key (hover over a mesh island); Ctrl+L (select all linked to current selection)

Selects all mesh elements topologically connected to the hovered or selected element. Useful for selecting an entire disconnected sub-mesh within the same object (e.g. a wheel that was merged into a car body). Implementation: BFS/DFS from the seed element through shared edges/faces, adding everything reachable to the selection set.

---

### Select all / deselect
**Shortcut:** A key (toggle)

First press selects all elements in the current mode (all vertices, all edges, or all faces). Second press deselects all. A simple but frequently used shortcut — omitting it forces the user to box-select the entire mesh, which is slower.

---

### Invert selection
**Shortcut:** Ctrl+I

Flips the selection: every currently selected element is deselected and every unselected element is selected. Useful when it is easier to select what you *don't* want and then invert, rather than selecting what you do want (e.g. select a small region you want to keep, invert, delete the rest).

---

### Hide / unhide selected
**Shortcuts:** H to hide selected elements; Alt+H to unhide all

Temporarily removes selected mesh elements from the viewport display *without* deleting them. Hidden elements are excluded from rendering, picking, and transform operations but remain in the mesh data. Useful for reducing visual clutter when working in a dense mesh — hide the polygons you're not currently editing to see the ones you are. Hidden elements are indicated in the data with a flag (`bool hidden` per element) and are restored on Alt+H or on exiting Edit Mode.

---

### Search / command palette
**Shortcut:** F3

A fuzzy-search text input that appears as a floating panel. Typing any part of an operation's name filters a list of all available editor operations and menu actions. Selecting one from the list executes it immediately. Makes any operation accessible with a few keystrokes without memorizing every menu location. Implementation: collect all registered operations (menu items, tool activations) into a flat list with their names; on F3, render an ImGui popup with an `InputText` and a filtered `ListBox`. Fuzzy-match using a simple substring or edit-distance scorer.

---

## Rendering & Display

### Viewport shading modes
**Shortcut:** Z key opens a pie menu; or buttons in the viewport header

Four progressive shading levels the user can switch between:
- **Wireframe** — only edge lines, no faces. Already implemented as a per-mesh toggle; this makes it a global viewport mode.
- **Solid** — flat-lit, single diffuse color per object. Fast, good for topology inspection. No textures, no PBR.
- **Material Preview** — full material parameters and lighting, but uses a built-in HDR environment (not scene lights). What Blender calls "LookDev".
- **Rendered** — full scene lighting, shadows, and post-processing. The authoritative preview of the final image.

Each mode corresponds to a different shader and FBO pipeline. The current app is stuck between Solid and Material Preview. The pie menu (or header buttons) writes a `ViewportShadingMode` enum to `EditorLayer`; the render path reads it each frame.

---

### SSAO
**Post-process pass:** Rendered after the geometry pass into an intermediate texture

Screen-Space Ambient Occlusion approximates how much ambient light a surface point receives based on its local geometry, by sampling nearby depth values in screen space. Points in crevices or corners that are surrounded by other geometry receive less ambient light and are darkened.

Implementation (standard Alchemy/HBAO approach):
1. Geometry pass: render world-space normals and depth into a G-buffer.
2. SSAO pass: for each screen pixel, sample N random points in a hemisphere oriented to the surface normal, project them into screen space, compare their depth against the depth buffer, and count how many are occluded. The ratio is the AO factor.
3. Blur pass: blur the raw AO texture (which is noisy) with a bilateral filter that preserves edges.
4. Composite: multiply scene ambient by the AO factor.

The sample hemisphere, sample count (16–64), and radius are configurable. SSAO significantly improves perceived depth and contact shadows at low cost.

---

### Bloom
**Post-process pass:** Applied after the main lit render

Makes bright surfaces appear to glow by adding a soft halo around them. Physically, real lenses scatter light from very bright sources onto neighboring pixels. Bloom also makes emissive materials feel self-luminous.

Standard implementation:
1. **Bright-pass threshold** — render a texture containing only pixels above a luminance threshold (e.g. HDR values > 1.0).
2. **Gaussian blur** — blur the bright-pass texture with a two-pass separable Gaussian kernel (horizontal then vertical), at several progressively larger scales (mipmaps or successive downsamples).
3. **Additive composite** — add the blurred result to the main HDR framebuffer before tone mapping.

Configurable parameters: threshold, blur radius, intensity (how much bloom is added). Bloom is visible on emissive materials and on any PBR surface with a bright specular highlight.

---

### FXAA / MSAA
**Preference:** Anti-aliasing mode selector (None / MSAA 2× / MSAA 4× / FXAA)

Reduces the staircase artifacts ("jaggies") on polygon edges.

- **MSAA (Multi-Sample Anti-Aliasing)** — renders each pixel multiple times at sub-pixel offsets and averages the results. Handled natively by OpenGL: create the framebuffer with `GL_TEXTURE_2D_MULTISAMPLE` and the required sample count. Very effective on geometry edges but does not smooth shader-internal aliasing (e.g. specular highlights). Expensive at 4×+ (4× the fragment shader invocations per pixel).
- **FXAA (Fast Approximate Anti-Aliasing)** — a post-process full-screen shader that detects and blurs high-contrast edges in the final image. Implemented as one extra `glDrawArrays` pass over a screen quad with the FXAA shader (a well-known ~100-line GLSL shader). Cheaper than MSAA, works on all aliasing sources, but slightly blurs the whole image.

For a learning project, FXAA is the simpler starting point (no framebuffer format changes needed, just a shader pass).

---

### Overlay toggles
**UI:** Overlay dropdown in the viewport header (gear/overlay icon)

Individual toggles for debug visualization layers that render on top of the scene geometry, useful when working in Edit Mode or checking model quality:

- **Face normals** — draw a short line from each face center in the direction of the face normal. Immediately shows flipped or inconsistent normals.
- **Vertex normals** — draw a line from each vertex in its normal direction. Useful for checking normal map baking targets.
- **Edge creases** — highlight edges with a non-zero crease weight (for Subdivision Surface modifier) in a distinct color.
- **Vertex weights** — color-map vertex weight values (used by modifiers or rigging) from blue (0) to red (1).
- **Face centers** — draw a dot at each face's centroid. Useful for verifying face orientation and face count.

All overlays are drawn as line or point geometry with `glDrawArrays(GL_LINES / GL_POINTS)` after the main scene, before the UI.

---

### Measurement overlay
**UI:** Overlay dropdown toggle; active only in Edit Mode

Displays metric labels next to selected geometry elements:
- **Edge length** — the Euclidean distance between the two endpoints of a selected edge, drawn in viewport units (meters by default, configurable to cm or inches).
- **Face area** — the area of a selected face polygon (computed using the cross-product of diagonals for planar faces; summed triangles for n-gons).
- **Angle** — the angle between two selected edges sharing a vertex, or the dihedral angle between two adjacent faces sharing an edge.

Labels are rendered as 2D text projected from the 3D element position to screen space (project the world-space midpoint through the view-projection matrix, then draw with ImGui's draw list or a text-render pass). Useful for precision modeling when exact dimensions matter.

---

## Tooling & UX

### Preferences panel
**Menu:** Edit > Preferences (Ctrl+,)

A persistent settings panel covering global editor behavior that does not belong to any scene:

- **Theme** — light/dark/custom color scheme for the ImGui UI.
- **Font size** — global ImGui font scale.
- **Keymap** — shows all current shortcut bindings (a read-only list here; edit via the Keymap editor).
- **Autosave interval** — how often (in minutes) the autosave file is written. 0 = disabled.
- **Undo history depth** — max entries in the undo stack (currently hardcoded at 100).
- **Snap increments** — default translate/rotate/scale snap values.
- **Shadow map resolution** — 512 / 1024 / 2048 / 4096 px.

Preferences are serialized to a JSON file in the user's config directory (e.g. `~/.config/opengl-modeler/prefs.json`) and loaded at startup.

---

### Keymap editor
**UI:** Section within Preferences → Keymap tab

Lists every registered editor action with its current keyboard binding. The user can click a binding, then press a new key combination to reassign it. Conflicting bindings are highlighted with a warning. "Reset to defaults" restores the built-in keymap.

Implementation: instead of hardcoding `if (key == GLFW_KEY_T)` checks throughout `EditorLayer`, introduce a `KeymapRegistry` — a `flat_map<ActionID, KeyBinding>`. Event handlers check `keymap.IsPressed(ActionID::TranslateGizmo, event)` instead of raw key codes. The Preferences UI reads and writes the same registry. The registry serializes to the prefs JSON file.

---

### Autosave
**Background timer:** fires every N minutes while the application runs

Writes a timestamped recovery copy of the current scene to a temp directory (e.g. `~/.local/share/opengl-modeler/autosave/autosave_<timestamp>.json`) without user interaction. On the next launch, if an autosave newer than the last manual save is found, a dialog offers to restore it. Prevents loss of work on crash.

Implementation: a simple timer in `Application::OnUpdate` tracks elapsed time. When the interval elapses it calls the scene serializer (same code path as File > Save) but writes to the autosave path. The recovery check runs during `Application::Init` before the first frame.

---

### Recent files
**Menu:** File > Open Recent → submenu listing the last N files

Stores a list of recently opened scene file paths (typically the last 10) and displays them in the File menu for one-click reopening. The list is persisted in the preferences JSON file alongside other settings. Each entry shows the filename and the full path as a tooltip. When a path no longer exists on disk it is shown grayed out and clicking it shows an error rather than crashing.

---

### Drag-and-drop asset import
**Interaction:** Drop a file onto the viewport window

Intercepts the OS file-drop event (GLFW provides `glfwSetDropCallback`) and dispatches to the appropriate loader based on file extension:
- `.obj`, `.fbx`, `.gltf` → import as a new mesh entity placed at the 3D cursor or world origin.
- `.png`, `.jpg`, `.hdr` → open a dialog asking whether to assign the texture to the selected object's material or load it as a new environment map.

No file dialog needed — the user drags directly from their file manager. The callback receives a `const char**` array of paths; each path is handed to `AssetManager::Load()`. If the extension is unknown, a toast notification explains the unsupported format.
