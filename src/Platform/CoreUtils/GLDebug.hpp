// OpenGL error utilities: GLCall macro clears GL errors before a call and logs any that result,
// with file/line info.
#pragma once
#include "Log.hpp"
#include <GL/glew.h>

#define DEBUG_BREAK() __builtin_trap()

// A macro to wrap our GL calls in for easy debugging
#define ASSERT(x)            \
    if (!(x)) DEBUG_BREAK();

// Main debug macro
#define GLCall(x)                                  \
    do {                                           \
        GLClearError();                            \
        x;                                         \
        ASSERT(GLLogCall(#x, __FILE__, __LINE__)); \
    } while (0)

void GLClearError();
bool GLLogCall(char const* function, char const* file, int line);
