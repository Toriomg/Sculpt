#pragma once

#include "glhead.h"
#include "Platform/CoreUtils/Math/maths.h"

class Mesh;
class Material;
class Camera;

// Static class, acts as a global service
class Renderer {
public:
    static void Init();
    static void Shutdown();

    // Call this at the start of every frame
    static void BeginScene(const Camera& camera);
    static void EndScene();

    // The main function to draw an object
    static void Submit(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material, const Vec4& transform);

    // A function to handle window resizing
    static void OnWindowResize(uint32_t width, uint32_t height);
};