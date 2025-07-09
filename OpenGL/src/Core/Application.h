#pragma once

#include <string>

#include "../Graphics/Renderer.h"
#include "../tests/tests.h" // For test::Test and test::TestMenu
#include "../Editor/EditorUI.h" // For EditorUI

// Forward declare GLFWwindow to avoid including glfw3.h in the header.
// This is good practice to keep headers clean and reduce compile times.
struct GLFWwindow;

class Application
{
public:
    Application(const std::string& name = "3d MODELER",
        unsigned int width = 1470, unsigned int height = 810);
    ~Application();

    void Run();

private:
    void Cleanup(); // Helper function for destructor

private:
    GLFWwindow* m_Window;
    std::string m_AppName;
    std::unique_ptr<EditorUI> m_EditorUI;
    unsigned int m_Width, m_Height;

    Renderer m_Renderer;

    // Test framework members
    test::Test* m_CurrentTest;
    test::TestMenu* m_TestMenu;
};