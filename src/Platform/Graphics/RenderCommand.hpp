#pragma once

#include "Platform/Graphics/Buffers/VertexArray.hpp"
#include "Platform/Graphics/Buffers/IndexBuffer.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include "Core/glhead.hpp"

// RenderCommand is a singleton class that encapsulates OpenGL rendering commands.
class RenderCommand {
public:
    static void Init();
    static void SetClearColor(const Vec4& color);
    static void Clear();

    // The main draw call
    static void Draw(
        const std::shared_ptr<VertexArray>& vertexArray,
        const std::shared_ptr<IndexBuffer>& indexBuffer
    );

    // Other useful commands
    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
};