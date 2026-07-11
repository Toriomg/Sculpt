// Infinite multi-scale grid rendered via ray-casting to the Y=0 plane; owned by EditorLayer.
#pragma once
#include <memory>
#include "Platform/Graphics/Shader.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"

class InfGrid {
public:
    InfGrid();
    ~InfGrid();

    InfGrid(const InfGrid&) = delete;
    InfGrid& operator=(const InfGrid&) = delete;
    InfGrid(InfGrid&&) = default;
    InfGrid& operator=(InfGrid&&) = default;

    // Draws the grid using the camera's separate view and projection matrices.
    // Must be called after the main scene geometry is already in the depth buffer.
    void Draw(const Matx4f& view, const Matx4f& projection, const Vec3& cameraPos) const;

private:
    std::unique_ptr<Shader> m_Shader;
    // Empty VAO required by the OpenGL Core Profile for attribute-less draw calls.
    unsigned int m_VAO = 0;
};
