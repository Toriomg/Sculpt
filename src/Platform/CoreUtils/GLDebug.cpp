#include "GLDebug.hpp"

void GLClearError() {
    // Keep getting errors until there are no more
    while (glGetError() != GL_NO_ERROR) {;
}
}

bool GLLogCall(char const* function, char const* file, int line) {
    while (GLenum error = glGetError()) {
        // Print an error message
        CORE_LOG_CRITICAL("[OpenGL Error] ({0}): {1} {2}:{3}", error, function, file, line);
        return false;  // Error occurred
    }
    return true;  // No error
}
