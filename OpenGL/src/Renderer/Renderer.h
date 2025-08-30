#pragma once

#include "glhead.h"
#include "Platform/CoreUtils/Math/maths.h"
#include "Platform/Graphics/RenderCommand.h"

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

    // A function to handle window resizing
    static void OnWindowResize(uint32_t width, uint32_t height);
};