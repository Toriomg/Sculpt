<h1 align="center">Sculpt</h1>

<p align="center">A 3D mesh modeling application built in C++23 with OpenGL.</p>

<img src="res/Screenshots/scr2.png" alt="project-screenshot" width="1462" height="805">
<img src="res/Screenshots/scr1.png" alt="project-screenshot" width="1462" height="805">

## What it does

Sculpt is a desktop 3D modeler with an ECS-based scene graph (EnTT), a custom math library, and a GPU picking system. You can create and edit 3D meshes, apply transforms, and import external assets.

**Object mode**
- Add 8 mesh primitives: Cube, Sphere, Pyramid, Torus, Cone, Arrow, Dodecahedron, Icosahedron
- Click to select objects; Shift+click to multi-select, also can change the pivot point
- Transform gizmos: **T** (translate), **R** (rotate) with per-axis handles, **S** (Scale). **G** (Global) for gizmo relation
- Undo / redo up to 100 steps — Ctrl+Z / Ctrl+Y
- Orbit camera: <!-- Click derecho, rota la camara, WASD, shift Space la traslada por el espacio -->
- Wireframe toggle per mesh
- Import file meshes and `.png`/`.jpg` textures

**Edit mode** (Tab to enter/exit)
- Select individual vertices, edges, or faces (keys **1** / **2** / **3**)
- Shift+click for additive element selection
- **E** — extrude selected faces, edges, or vertices
- More in development...

**UI**
- Outliner, Inspector, Viewport, and Menu bar panels (ImGui)
- Infinite Y=0 grid via ray-cast shader
- Light and Dark Mode

## Build & run

```bash
cmake -B build && cmake --build build
./build/Sculpt
```

Requires: CMake, a C++23-capable Clang, OpenGL 4.x, GLFW, GLEW, Assimp.
