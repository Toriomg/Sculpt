#include "Platform/Application.hpp"

// Automatically load LSAN suppressions in Debug builds so system-library leaks
// (libdecor-gtk, libpango, libfontconfig via GLFW Wayland decorations) don't
// drown out real leaks. No LSAN_OPTIONS environment variable needed.
#ifdef LSAN_SUPPRESSIONS_PATH
extern "C" char const* __lsan_default_options() {
    return "suppressions=" LSAN_SUPPRESSIONS_PATH;
}
#endif

int main(void) {
    // Create an instance of the application on the stack
    Application app("My 3D Modeling Program", 1470, 810);

    // Run the application's main loop
    app.Run();

    return 0;
}
