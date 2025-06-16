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
#include "Shader.h"
#include "Texture.h"

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

        float positions[] = {
            -50.0f, -50.0f, 0.0f, 0.0f, // Bottom left
             50.0f, -50.0f, 1.0f, 0.0f, // Bottom right
             50.0f,  50.0f, 1.0f, 1.0f,  // Top right
            -50.0f,  50.0f, 0.0f, 1.0f // Top left
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

		GLCall(glEnable(GL_BLEND)); // Enable blending for transparency
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Set the blending function for transparency

		VertexArray va; // Create a Vertex Array Object (VAO) to hold the vertex attributes
        VertexBuffer vb(positions, sizeof(positions)); // Create a Vertex Buffer Object (VBO) with the vertex data

        VertexBufferLayout layout;
		layout.Push<float>(2); 
        layout.Push<float>(2);
		va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, sizeof(indices)); // Create an Index Buffer Object (IBO) with the index data

        glm::mat4 proj = glm::ortho(0.0f, WINDW_SIZE_X, 0.0f, WINDW_SIZE_Y, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

        glm::vec4 vp(100.0f, 100.0f, 0.0f, 1.0f);

        
        // Parse the shader file
		Shader shader("res/shaders/Basic.shader"); // Create a Shader object with the shader file path
		shader.Bind(); // Bind the shader program

        Texture texture("res/textures/texture1.png");
		texture.Bind(); // Bind the texture 
		shader.SetUniform1i("u_Texture", 0); // Set the texture uniform in the shader

		va.Unbind(); // Unbind the VAO
		shader.Unbind(); // Unbind the shader program
		vb.Unbind(); // Unbind the VBO
		ib.Unbind(); // Unbind the IBO

		Renderer renderer; // Create a Renderer object to handle OpenGL calls

		// Setting up ImGui
		IMGUI_CHECKVERSION(); // Check ImGui version
		ImGui::CreateContext(); // Initialize ImGui context
		ImGuiIO& io = ImGui::GetIO(); // Get ImGui IO object
		(void)io; // Suppress unused variable warning
		ImGui::StyleColorsDark(); // Set ImGui style to dark
		ImGui_ImplGlfw_InitForOpenGL(window, true); // Initialize ImGui for GLFW
        ImGui_ImplOpenGL3_Init("#version 330"); // Initialize ImGui for OpenGL 3.3

        glm::vec3 translationA(200.0f, 200.0f, 0.0f);
        glm::vec3 translationB(400.0f, 200.0f, 0.0f);

        float r = 0.2f;
        float g = 0.1f;
        float b = 0.2f;
        float a = 1.0f;
        float var = 0.01f; // Variable to control color change
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplOpenGL3_NewFrame(); // Start a new ImGui frame
            ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame(); // Create a new ImGui frame


			shader.Bind(); // Bind the shader program
            {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), translationA);
                glm::mat4 mvp = proj * view * model;
                shader.SetUniformMat4f("u_MVP", mvp);
                // Draw the object using the Renderer
                renderer.Draw(va, ib, shader);
            }

            {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), translationB);
                glm::mat4 mvp = proj * view * model;
                shader.SetUniformMat4f("u_MVP", mvp);
                // Draw the object using the Renderer
                renderer.Draw(va, ib, shader);
            }


            if (r >= 1.0f) {
                var = -0.01f; // Reverse direction when reaching 1.0
            }
            else if (r <= 0.0f) {
                var = 0.01f; // Reverse direction when reaching 0.0
            }
            r += var; // Increment the red component

            {
                ImGui::Begin("Debug");
                ImGui::Text("Hello, world!"); // Display text in the ImGui window
                ImGui::SliderFloat3("translation a", &translationA.x, 0.0f, WINDW_SIZE_X);
                ImGui::SliderFloat3("translation b", &translationB.x, 0.0f, WINDW_SIZE_X);
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                ImGui::End(); // End the ImGui window
            }

			ImGui::Render(); // Render ImGui
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render ImGui draw data

            /* Swap front and back buffers */
            glfwSwapBuffers(window);
            /* Poll for and process events */
            glfwPollEvents();
        }
    }

	ImGui_ImplOpenGL3_Shutdown(); // Shutdown ImGui for OpenGLç
    ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(); // Destroy ImGui context

    glfwTerminate();
    return 0;
}