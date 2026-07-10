#pragma once
#include <GL/glew.h>
#include "Log.h"

// Use a debug break that works on multiple compilers
#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define DEBUG_BREAK() __builtin_trap()
#else
#define DEBUG_BREAK()
#endif

// A macro to wrap our GL calls in for easy debugging
#define ASSERT(x) if (!(x)) DEBUG_BREAK();

// Main debug macro
#define GLCall(x) do { GLClearError(); x; ASSERT(GLLogCall(#x, __FILE__, __LINE__)); } while(0)

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);