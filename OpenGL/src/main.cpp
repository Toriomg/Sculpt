#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h" // Include the Renderer header for GLCall macro

#include "buffers/VertexBuffer.h"
#include "buffers/VertexBufferLayout.h"
#include "buffers/IndexBuffer.h"
#include "buffers/VertexArray.h"
#include "Utils/Shader.h"
#include "Utils/Texture.h"
#include "tests/TestClearColor.h"
#include "tests/TestTexture2D.h"
#include "tests/TestBatchRendering.h"
#include "tests/test3.h"

#include "glm/glm.hpp" // Include GLM for vector and matrix operations
#include "glm/gtc/matrix_transform.hpp" // Include GLM for matrix transformations

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

const float WINDW_SIZE_X = 960.0f; // Define the window width
const float WINDW_SIZE_Y = 540.0f; // Define the window height

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set the major version of OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set the minor version of OpenGL
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use the core profile of OpenGL

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow((int)WINDW_SIZE_X, (int)WINDW_SIZE_Y, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	glfwSwapInterval(1); // Enable vsync

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

	std::cout << glGetString(GL_VERSION) << std::endl;
    {
        GLCall(glEnable(GL_BLEND)); // Enable blending for transparency
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Set the blending function for transparency

		Renderer renderer; // Create a Renderer object to handle OpenGL calls

		// Setting up ImGui
		IMGUI_CHECKVERSION(); // Check ImGui version
		ImGui::CreateContext(); // Initialize ImGui context
		ImGuiIO& io = ImGui::GetIO(); // Get ImGui IO object
		(void)io; // Suppress unused variable warning
		ImGui::StyleColorsDark(); // Set ImGui style to dark
		ImGui_ImplGlfw_InitForOpenGL(window, true); // Initialize ImGui for GLFW
        ImGui_ImplOpenGL3_Init("#version 330"); // Initialize ImGui for OpenGL 3.3

        test::Test* currentTest = nullptr;
		test::TestMenu* testMenu = new test::TestMenu(currentTest);
		currentTest = testMenu; // Set the current test to the test menu

        testMenu->RegisterTest<test::TestClearColor>("Clear Color");
        testMenu->RegisterTest<test::TestTexture2D>("2D Textures");
        testMenu->RegisterTest<test::TestBatchRendering>("Batch Rendering");   
        testMenu->RegisterTest<test::test3>("Batch Rendering with my maths");

        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
			renderer.Clear(); // Clear the screen

			ImGui_ImplOpenGL3_NewFrame(); // Start a new ImGui frame
			ImGui_ImplGlfw_NewFrame(); // Start a new ImGui frame for GLFW
			ImGui::NewFrame(); // Create a new ImGui frame
			
			if (currentTest) {
                currentTest->OnUpdate(0.0f); // Update the current test object
                currentTest->OnRender(); // Render the current test object

				// Render test UI
				ImGui::Begin("Test Menu"); // Begin the ImGui window for the test menu
                if (currentTest != testMenu && ImGui::Button("<-")) {
                    delete currentTest;
                    currentTest = testMenu;
                }
				currentTest->OnImGuiRender(); // Render the ImGui interface for the current test
                ImGui::End();
            }


			ImGui::Render(); // Render ImGui
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render ImGui draw data

            /* Swap front and back buffers */
            glfwSwapBuffers(window);
            /* Poll for and process events */
            glfwPollEvents();
        }
        if (currentTest) {
            delete currentTest; // This will delete testMenu if it's the active test
            currentTest = nullptr; // Avoid dangling pointer
        }
    }

	ImGui_ImplOpenGL3_Shutdown(); // Shutdown ImGui for OpenGL
    ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(); // Destroy ImGui context

    glfwTerminate();
    return 0;
}