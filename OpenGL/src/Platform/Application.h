#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glhead.h"
#include "System/Time.h"
#include "System/Input.h"

class Application
{
public:
    Application(const std::string& name = "3d MODELER",
        unsigned int width = 1470, unsigned int height = 810);
    ~Application();

    void Run();

private:
    GLFWwindow* m_Window;
    std::string m_AppName;
    unsigned int m_Width, m_Height;
};