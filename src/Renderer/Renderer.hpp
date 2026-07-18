// Static high-level rendering API: BeginScene sets the view-projection, Submit draws a
// Mesh+Material pair.
#pragma once

#include "Core/glhead.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include "Platform/Graphics/RenderCommand.hpp"

class Mesh;
class Material;
class Camera;
class Shader;

// Static class, acts as a global service
class Renderer {
public:
    static void Init();
    static void Shutdown();

    static void BeginScene(Matx4f const& ViewProjectionMatrix);
    static void EndScene();

    // The main function to draw an object
    static void Submit(std::shared_ptr<Mesh> const& mesh, std::shared_ptr<Material> const& material,
                       Matx4f const& transform);

    // Three-pass wireframe: black fill → white edges → white vertex dots
    static void SubmitWireframe(std::shared_ptr<Mesh> const& mesh, Matx4f const& transform);

    // Single-pass flat-color draw reusing WireframeShader. For editor overlays (gizmos, debug).
    static void SubmitFlat(std::shared_ptr<Mesh> const& mesh, Vec4 const& color,
                           Matx4f const& transform);

    // Draws a screen-space outline around the mesh using the outline shader.
    // Caller is responsible for setting up stencil state before and after.
    static void SubmitOutline(std::shared_ptr<Mesh> const& mesh, Vec4 const& color, float thickness,
                              Matx4f const& transform);

    // Edit mode overlay: dark edge lines + orange vertex dots on top of the solid mesh.
    static void SubmitEditOverlay(std::shared_ptr<Mesh> const& mesh, Matx4f const& transform);

    static void OnWindowResize(uint32_t width, uint32_t height);

    static void SetDebugSelectionMode(bool enable);
    static bool IsDebugSelectionModeEnabled();
    static std::shared_ptr<Shader> const& GetDebugSelectionShader();

    static void SetSceneClearColor(Vec4 const& color);
    static Vec4 GetSceneClearColor();

    static void SetGridColor(Vec3 const& color);
    static Vec3 GetGridColor();
};
