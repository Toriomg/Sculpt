#include "Platform/Application.hpp"
#include <iostream>

int main(void)
{
    // Create an instance of the application on the stack
    Application app("My 3D Modeling Program", 1470, 810);

    // Run the application's main loop
    app.Run();

    return 0;
}