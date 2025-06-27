#pragma once
#include <memory>

// Macro to assert conditions, triggering a breakpoint if false
#define ASSERT(x) if (!(x)) __debugbreak() 
// Macro to clear OpenGL errors before and after a function call
#define GLCall(x) GLClearError(); x; ASSERT(GLLogCall(#x, __FILE__, __LINE__)) 
// Macro to log OpenGL errors
#define BIT(x) (1 << (x))