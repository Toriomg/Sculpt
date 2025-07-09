#include "Time.h"
#include <GLFW/glfw3.h> // We use GLFW to get the high-resolution time

// Initialize the static member variables
float Time::s_LastFrameTime = 0.0f;
float Time::s_Time = 0.0f;
float Time::s_DeltaTime = 0.0f;

void Time::Init()
{
    // Get the initial time when the application starts
    s_LastFrameTime = (float)glfwGetTime();
    s_Time = s_LastFrameTime;
    s_DeltaTime = 0.0f;
}

void Time::Update()
{
    // Get the current time from GLFW
    float currentTime = (float)glfwGetTime();

    // Calculate the difference in time from the last frame
    s_DeltaTime = currentTime - s_LastFrameTime;

    // Update the last frame time for the next iteration
    s_LastFrameTime = currentTime;

    // Update the total elapsed time
    s_Time = currentTime;
}