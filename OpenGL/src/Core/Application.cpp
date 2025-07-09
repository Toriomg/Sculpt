// src/Core/Application.cpp
#include "Application.h"

// --- All the includes that were in main.cpp ---
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "InputManager.h" // Assuming this is in Core now

// Test implementations
#include "../tests/TestClearColor.h"
#include "../tests/TestTexture2D.h"
#include "../tests/TestBatchRendering.h"
#include "../tests/test3.h"

// ImGui
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
// ---------------------------------------------


Application::Application(const std::string& name, unsigned int width, unsigned int height)
    : m_Window(nullptr), m_AppName(name), m_Width(width), m_Height(height),
    m_CurrentTest(nullptr), m_TestMenu(nullptr)
{
    /* Initialize the library */
    if (!glfwInit()) {
        std::cerr << "ERROR: glfwInit() failed." << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    m_Window = glfwCreateWindow(m_Width, m_Height, m_AppName.c_str(), NULL, NULL);
    if (!m_Window)
    {
        glfwTerminate();
        std::cerr << "ERROR: glfwCreateWindow() failed." << std::endl;
        return;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1); // Enable vsync

    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: glewInit() failed." << std::endl;
        return;
    }

    InitializeInput(m_Window); // Initialize your input handling
    std::cout << glGetString(GL_VERSION) << std::endl;

    // --- Setup OpenGL State ---
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glEnable(GL_CULL_FACE));
    GLCall(glFrontFace(GL_CCW));
    GLCall(glCullFace(GL_BACK));

    // --- Setup ImGui ---
    m_EditorUI = std::make_unique<EditorUI>(m_Window);

    // --- Setup Test Framework ---
    m_TestMenu = new test::TestMenu(m_CurrentTest);
    m_CurrentTest = m_TestMenu;

    m_TestMenu->RegisterTest<test::TestClearColor>("Clear Color");
    m_TestMenu->RegisterTest<test::TestTexture2D>("2D Textures");
    m_TestMenu->RegisterTest<test::TestBatchRendering>("Batch Rendering");
    m_TestMenu->RegisterTest<test::test3>("Batch Rendering with my maths");
}

void Application::Run()
{
    float lastTime = 0.0f;
    while (!glfwWindowShouldClose(m_Window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        /* Render here */
        m_Renderer.Clear();

        m_EditorUI->BeginFrame();

        if (m_CurrentTest) {
            m_CurrentTest->OnUpdate(deltaTime);
            m_CurrentTest->OnRender();
            m_CurrentTest->OnInput(m_Window, deltaTime);

            ImGui::Begin("Test Menu");
            // If the current test is not the menu, show the "back" button
            if (m_CurrentTest != m_TestMenu && ImGui::Button("<- Back to Menu")) {
                delete m_CurrentTest; // Delete the old test
                m_CurrentTest = m_TestMenu; // Go back to the menu
            }
            m_CurrentTest->OnImGuiRender();
            ImGui::End();
        }

        // Render the ImGui frame
        m_EditorUI->EndFrame();

        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}

void Application::Cleanup()
{
    // Important: Clean up the current test if it's not the menu itself,
    // then clean up the menu. This prevents a memory leak.
    if (m_CurrentTest != m_TestMenu)
        delete m_CurrentTest;
    delete m_TestMenu;


    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

Application::~Application()
{
    Cleanup();
}