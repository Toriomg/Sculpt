#pragma once

#include <GL/glew.h> // Include GLEW for OpenGL function pointers

#include "../Buffers/VertexArray.h"
#include "../Buffers/IndexBuffer.h"
#include "../Shading/Shader.h"



class RenderCommand {
public:
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
	void Clear() const;
};