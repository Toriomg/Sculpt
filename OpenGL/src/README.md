# OpenGL/src Directory Overview

This directory contains the source code for the OpenGL project, organized as follows:

- **glhead.h, main.cpp**: Entry point and global headers.
- **README.md**: Project documentation.

## Application
TODO:
- Contains application-specific code.
    - **Editor**: Placeholder for editor-related features.

## Core
TODO:
- Fundamental components and systems for the engine.
    - **Components**: Entity/component definitions.
    - **Systems**: Logic and processing systems.

## Platform
- Platform abstraction and utilities.
    - **Application.cpp/h**: Application lifecycle management.
    - **CoreUtils**: Utility functions.
        - **Log.cpp/h**: Logging utilities.
        - **Math**: Math-related headers and implementations (vectors, matrices, quaternions).
    - **Graphics**: Rendering abstractions.
        - **Framebuffer, RenderCommand, Shader, Texture, Vertex**: Graphics pipeline objects.
        - **Buffers**: Buffer management (vertex, index, layouts).
    - **Layers**: Layered architecture for modularity.
        - **EditorLayer, Layer, LayerStack**: Layer management.
    - **System**: Input, timing, and windowing.
        - **Input.cpp/h, Time.cpp/h**: Input handling and timing.
        - **Events**: Event system (keyboard, mouse, window).
        - **Input**: Input abstraction (GLFW integration, key codes).
        - **Window**: Window management (GLFW integration).

## Renderer
- Rendering engine and assets.
    - **Material.h, Mesh.h, Renderer.cpp/h**: Rendering logic and asset definitions.
    - **Assets**: Asset management.

## vendor
- Third-party dependencies.
    - **ImGui**: Debug/UI menus
    - **spdlog**: Logging
    - **stb_image**: Image rendering for textures

---
Each folder and file is structured to separate concerns, improve maintainability, and support cross-platform development.

- MAINLY AI GENERATED, CHECKED AND MODIFIED MANUALLY.