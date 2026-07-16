// Infinite multi-scale grid rendered via ray-casting to the Y=0 plane; owned by EditorLayer.
#pragma once
#include "Platform/CoreUtils/Math/maths.hpp"
#include "Platform/Graphics/Shader.hpp"
#include <memory>

class InfGrid {
public:
    InfGrid();
    ~InfGrid();

    InfGrid(InfGrid const&)            = delete;
    InfGrid& operator=(InfGrid const&) = delete;
    InfGrid(InfGrid&&)                 = default;
    InfGrid& operator=(InfGrid&&)      = default;

    // Draws the grid using the camera's separate view and projection matrices.
    // Must be called after the main scene geometry is already in the depth buffer.
    void Draw(Matx4f const& view, Matx4f const& projection, Vec3 const& cameraPos) const;

private:
    std::unique_ptr<Shader> m_Shader;
    // Empty VAO required by the OpenGL Core Profile for attribute-less draw calls.
    unsigned int m_VAO = 0;
};
