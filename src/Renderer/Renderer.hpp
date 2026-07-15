// Static high-level rendering API: BeginScene sets the view-projection, Submit draws a Mesh+Material pair.
#pragma once

#include "Core/glhead.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include "Platform/Graphics/RenderCommand.hpp"

class Mesh;
class Material;
class Camera;

// Static class, acts as a global service
class Renderer {
public:
    static void Init();
    static void Shutdown();

    static void BeginScene(const Matx4f& ViewProjectionMatrix);
    static void EndScene();

    // The main function to draw an object
    static void Submit(
        const std::shared_ptr<Mesh>& mesh,
        const std::shared_ptr<Material>& material,
        const Matx4f& transform
    );

    // Three-pass wireframe: black fill → white edges → white vertex dots
    static void SubmitWireframe(
        const std::shared_ptr<Mesh>& mesh,
        const Matx4f& transform
    );

    // Single-pass flat-color draw reusing WireframeShader. For editor overlays (gizmos, debug).
    static void SubmitFlat(
        const std::shared_ptr<Mesh>& mesh,
        const Vec4& color,
        const Matx4f& transform
    );

    static void OnWindowResize(uint32_t width, uint32_t height);
};