// Thin static wrapper over raw OpenGL calls: clear, viewport, and the indexed VAO+IBO draw call.
#pragma once

#include "Platform/Graphics/Buffers/VertexArray.hpp"
#include "Platform/Graphics/Buffers/IndexBuffer.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include "Core/glhead.hpp"

enum class DepthFunc { Less, Always };

// RenderCommand is a singleton class that encapsulates OpenGL rendering commands.
class RenderCommand {
public:
    static void Init();
    static void SetClearColor(const Vec4& color);
    static void Clear();
    static void SetDepthFunc(DepthFunc func);

    // Indexed triangle draw
    static void Draw(
        const std::shared_ptr<VertexArray>& vertexArray,
        const std::shared_ptr<IndexBuffer>& indexBuffer
    );

    // Non-indexed point draw (used by wireframe vertex pass)
    static void DrawPoints(const std::shared_ptr<VertexArray>& vertexArray, uint32_t vertexCount);

    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
};