#pragma once

#include <GL/glew.h> // Include GLEW for OpenGL function pointers

#include "buffers/VertexArray.h"
#include "buffers/IndexBuffer.h"
#include "Shader.h"

// Macro to assert conditions, triggering a breakpoint if false
#define ASSERT(x) if (!(x)) __debugbreak() 
// Macro to clear OpenGL errors before and after a function call
#define GLCall(x) GLClearError(); x; ASSERT(GLLogCall(#x, __FILE__, __LINE__)) 

void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

class Renderer {
public:
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
};