#pragma once
#include <GL/glew.h>
#include "Log.hpp"

#define DEBUG_BREAK() __builtin_trap()

// A macro to wrap our GL calls in for easy debugging
#define ASSERT(x) if (!(x)) DEBUG_BREAK();

// Main debug macro
#define GLCall(x) do { GLClearError(); x; ASSERT(GLLogCall(#x, __FILE__, __LINE__)); } while(0)

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);